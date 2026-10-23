namespace aurora::gx {
std::string petari_uber_source(DstAlphaMode mode,uint32_t normal) {
  // Reuse Aurora's byte-fetch and overflow WGSL verbatim; runtime entrypoints below
  // use the same storage buffers and immediate ABI.
  ShaderConfig seed{};seed.tevStageCount=1;seed.vtxStride=12;seed.attrs[GX_VA_POS]={.attrType=GX_DIRECT,.cnt=3,.compType=GX_F32};
  auto src=build_shader_source(seed,DstAlphaMode::None);src.resize(src.find("struct Immediate"));
  src+=petariUberWgsl;
  for(unsigned i=0;i<8;++i) src+=fmt::format("\n@group(2) @binding({}) var tex{}:texture_2d<f32>;\n@group(2) @binding({}) var samp{}:sampler;\n",i*2,i,i*2+1,i);
  src+="fn sample_tex(i:u32,uv:vec2f)->vec4f { switch(i) {";
  for(unsigned i=0;i<8;++i) src+=fmt::format("case {}u:{{return textureSampleBias(tex{},samp{},uv,ubuf.size_bias[{}].z);}}",i,i,i,i);
  src+="default:{return vec4f(1);}}}\n";
  for(bool alpha:{false,true}) {
    src+=alpha?"fn aop(o:u32,a:f32,b:f32,c:f32,d:f32)->f32 {var v=0.0;switch(o){":"fn cop(o:u32,a:vec3f,b:vec3f,c:vec3f,d:vec3f)->vec3f {var v=vec3f(0);switch(o){";
    for(unsigned op:{0u,1u,8u,9u,10u,11u,12u,13u,14u,15u}) {
      // Alpha comparison forms use scalar .r/.rg/.rgb in Aurora's generic helper;
      // only A8 compare (14/15) is a legal scalar form.
      if(alpha && op>=8 && op<14) continue;
      src+=fmt::format("case {}u:{{v={};}}",op,tev_op(static_cast<GXTevOp>(op),"","","a","b","c","d",alpha?"0.0":"vec3f(0)"));
    }
    src+="default:{}}return v;}\n";
  }
  src+=R"(
fn scale_bias(v:vec3f,b:u32,s:u32)->vec3f {var x=v;if(b==1u){x+=vec3f(0.5);}if(b==2u){x-=vec3f(0.5);}switch(s){case 1u:{x*=2.0;}case 2u:{x*=4.0;}case 3u:{x*=0.5;}default:{}}return x;}
fn run(in:VertexOutput)->vec4f {
 var r=ubuf.regs;var cn=array<bool,4>(false,false,false,false);var an=cn;
 let tc=array<vec3f,8>(in.tc0,in.tc1,in.tc2,in.tc3,in.tc4,in.tc5,in.tc6,in.tc7);
 var lastc=0u;var lasta=0u;
 for(var i=0u;i<cfg(822u);i++) {
 let o=i*32u;var t=vec4f(1);let map=cfg(o+21u);if(map<8u){let ti=cfg(o+20u);let coord=tc[ti];var uv=coord.xy;if(cfg(704u+ti*8u)==0u){uv=coord.xy/coord.z;}t=sample_tex(map,uv);}t=swizzle(t,cfg(o+24u));
 var ras=vec4f(0);let ch=cfg(o+22u);if(ch==0u || ch==2u || ch==4u){ras=in.cc0;}if(ch==1u || ch==3u || ch==5u){ras=in.cc1;}ras=swizzle(ras,cfg(o+23u));
 let k=konst(cfg(o+18u));let ka=konst(cfg(o+19u));
 var args:array<vec3f,4>;
 for(var j=0u;j<4u;j++){let a=cfg(o+j);var v=carg(a,r,t,ras,k);if(j<3u && a<8u){var norm=cn[a/2u];if((a&1u)!=0u){norm=an[a/2u];}if(!norm){v=tev_overflow_vec3f(v);}}args[j]=v;}
 var v=cop(cfg(o+8u),args[0],args[1],args[2],args[3]);if(cfg(o+8u)<2u){v=scale_bias(v,cfg(o+9u),cfg(o+10u));}let cl=cfg(o+11u)!=0u;v=clamp(v,vec3f(select(-4.0,0.0,cl)),vec3f(select(4.0,1.0,cl)));lastc=cfg(o+12u);r[lastc]=vec4f(v,r[lastc].a);cn[lastc]=cl;
 var aa:array<f32,4>;for(var j=0u;j<4u;j++){let a=cfg(o+4u+j);var x=aarg(a,r,t,ras,ka);if(j<3u && a<4u && !an[a]){x=tev_overflow_f32(x);}aa[j]=x;}
 var av=aop(cfg(o+13u),aa[0],aa[1],aa[2],aa[3]);if(cfg(o+13u)<2u){av=scale_bias(vec3f(av),cfg(o+14u),cfg(o+15u)).x;}let acl=cfg(o+16u)!=0u;lasta=cfg(o+17u);r[lasta].a=clamp(av,select(-4.0,0.0,acl),select(4.0,1.0,acl));an[lasta]=acl;
 }
 var color=r[lastc].rgb;var alpha=r[lasta].a;if(!cn[lastc]){color=tev_overflow_vec3f(color);}if(!an[lasta]){alpha=tev_overflow_f32(alpha);}
 let byte=u32(round(clamp(alpha,0.0,1.0)*255.0));let a=compare(byte,cfg(801u),cfg(800u));let b=compare(byte,cfg(803u),cfg(802u));var alpha_pass=false;switch(cfg(820u)){case 0u:{alpha_pass=a&&b;}case 1u:{alpha_pass=a||b;}case 2u:{alpha_pass=a!=b;}default:{alpha_pass=a==b;}}if(!alpha_pass){discard;}return vec4f(color,alpha);
}
)";
  if(mode==DstAlphaMode::DualSource) {
    src.insert(0,"enable dual_source_blending;\n");src+="struct Frag {@location(0) @blend_src(0) color:vec4f,@location(0) @blend_src(1) blend:vec4f,};@fragment fn fs_main(in:VertexOutput)->Frag {let c=run(in);return Frag(vec4f(c.rgb,1),c);}";
  } else {
    std::string ret="var c=run(in);";if(mode==DstAlphaMode::Replace)ret+="c.a=1.0;";
    if(normal==UINT32_MAX) src+="@fragment fn fs_main(in:VertexOutput)->@location(0) vec4f {"+ret+"return c;}";
    else src+=fmt::format("struct Frag {{@location(0) color:vec4f,@location({}) normal:vec4f,}};@fragment fn fs_main(in:VertexOutput)->Frag {{ {} let n=in.normal;let len=dot(n,n);var out=vec4f(0.5,0.5,0.5,0);if(cfg(823u)!=0u){{out=vec4f(select(vec3f(0),normalize(n),len>1e-10)*0.5+0.5,select(0.0,1.0,len>1e-10));}}return Frag(c,out);}}",normal,ret);
  }
  return src;
}
wgpu::ShaderModule petari_uber_shader(DstAlphaMode mode,uint32_t normal) {
 static std::mutex mutex;static std::map<std::pair<unsigned,uint32_t>,wgpu::ShaderModule> modules;
 std::lock_guard lock(mutex);auto key=std::make_pair(unsigned(mode),normal);auto it=modules.find(key);if(it!=modules.end())return it->second;
 auto start=std::chrono::steady_clock::now();auto src=petari_uber_source(mode,normal);
 PetariPipeline::dumpSource(xxh3_hash_s(src.data(),src.size()),src);
 wgpu::ShaderSourceWGSL wgsl{};wgsl.code=src.c_str();wgpu::ShaderModuleDescriptor desc{.nextInChain=&wgsl,.label="Petari ubershader"};auto module=webgpu::g_device.CreateShaderModule(&desc);
 auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();petariUberShaderMs+=ms;std::fprintf(stderr,"[gx uber shader] mode=%u normal=%u source_bytes=%zu shader_ms=%.3f\n",unsigned(mode),normal,src.size(),ms);
 modules.emplace(key,module);return module;
}
}
