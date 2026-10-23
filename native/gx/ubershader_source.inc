#include <tuple>
namespace aurora::gx {
// Expand loop bodies with literal indices. This prevents dynamic local-array
// indexing and gives Dawn/Metal constant configuration addresses per stage.
static void petari_uber_unroll(std::string& src,std::string_view loop,char variable,unsigned count) {
  auto begin=src.find(loop);if(begin==std::string::npos)std::abort();
  auto bodyBegin=begin+loop.size();auto end=bodyBegin;unsigned depth=1;
  while(end<src.size() && depth){if(src[end]=='{')++depth;if(src[end]=='}')--depth;++end;}
  if(depth)std::abort();
  auto body=src.substr(bodyBegin,end-bodyBegin-1);std::string expanded;
  for(unsigned n=0;n<count;++n){
    expanded+="{\n";
    for(size_t p=0;p<body.size();){
      auto ident=[](char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_';};
      if(ident(body[p])){auto q=p+1;while(q<body.size()&&ident(body[q]))++q;
        if(q==p+1 && body[p]==variable)expanded+=std::to_string(n)+"u";else expanded+=body.substr(p,q-p);p=q;
      }else expanded+=body[p++];
    }
    expanded+="\n}\n";
  }
  src.replace(begin,end-begin,expanded);
}
std::string petari_uber_source(DstAlphaMode mode,uint32_t normal,uint32_t variant) {
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
 var r=TevRegisters(ubuf.regs[0],ubuf.regs[1],ubuf.regs[2],ubuf.regs[3]);var cn=0u;var an=0u;
 var lastc=0u;var lasta=0u;
 for(var i=0u;i<cfg(822u);i++) {
 let o=i*32u;var t=vec4f(1);let map=cfg(o+21u);if(map<8u){let ti=cfg(o+20u);let coord=texcoord(in,ti);var uv=coord.xy;if(cfg(704u+ti*8u)==0u){uv=coord.xy/coord.z;}t=sample_tex(map,uv);}t=swizzle(t,cfg(o+24u));
 var ras=vec4f(0);let ch=cfg(o+22u);if(ch==0u || ch==2u || ch==4u){ras=in.cc0;}if(ch==1u || ch==3u || ch==5u){ras=in.cc1;}ras=swizzle(ras,cfg(o+23u));
 let k=konst(cfg(o+18u));let ka=konst(cfg(o+19u));
 var args:array<vec3f,4>;
 for(var j=0u;j<4u;j++){let a=cfg(o+j);var v=carg(a,r,t,ras,k);if(j<3u && a<8u){var norm=(cn&(1u<<(a/2u)))!=0u;if((a&1u)!=0u){norm=(an&(1u<<(a/2u)))!=0u;}if(!norm){v=tev_overflow_vec3f(v);}}args[j]=v;}
 var v=cop(cfg(o+8u),args[0],args[1],args[2],args[3]);if(cfg(o+8u)<2u){v=scale_bias(v,cfg(o+9u),cfg(o+10u));}let cl=cfg(o+11u)!=0u;v=clamp(v,vec3f(select(-4.0,0.0,cl)),vec3f(select(4.0,1.0,cl)));lastc=cfg(o+12u);switch(lastc){case 0u:{r.r0=vec4f(v,r.r0.a);}case 1u:{r.r1=vec4f(v,r.r1.a);}case 2u:{r.r2=vec4f(v,r.r2.a);}default:{r.r3=vec4f(v,r.r3.a);}}cn=(cn&~(1u<<lastc))|select(0u,1u<<lastc,cl);
 var aa:array<f32,4>;for(var j=0u;j<4u;j++){let a=cfg(o+4u+j);var x=aarg(a,r,t,ras,ka);if(j<3u && a<4u && (an&(1u<<a))==0u){x=tev_overflow_f32(x);}aa[j]=x;}
 var av=aop(cfg(o+13u),aa[0],aa[1],aa[2],aa[3]);if(cfg(o+13u)<2u){av=scale_bias(vec3f(av),cfg(o+14u),cfg(o+15u)).x;}let acl=cfg(o+16u)!=0u;lasta=cfg(o+17u);let alpha=clamp(av,select(-4.0,0.0,acl),select(4.0,1.0,acl));switch(lasta){case 0u:{r.r0.a=alpha;}case 1u:{r.r1.a=alpha;}case 2u:{r.r2.a=alpha;}default:{r.r3.a=alpha;}}an=(an&~(1u<<lasta))|select(0u,1u<<lasta,acl);
 }
 var color=read_reg(r,lastc).rgb;var alpha=read_reg(r,lasta).a;if((cn&(1u<<lastc))==0u){color=tev_overflow_vec3f(color);}if((an&(1u<<lasta))==0u){alpha=tev_overflow_f32(alpha);}
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
  petari_uber_unroll(src,"for(var j=0u;j<4u;j++){",'j',4);
  petari_uber_unroll(src,"for(var j=0u;j<4u;j++){",'j',4);
  petari_uber_unroll(src,"for(var i=0u;i<cfg(822u);i++) {",'i',variant&31u);
  petari_uber_unroll(src,"for(var i=0u;i<8u;i++) {",'i',(variant>>5)&15u);
  // The only remaining accesses to the argument arrays now have literal indices.
  auto replaceAll=[&](std::string_view from,std::string_view to){size_t pos=0;while((pos=src.find(from,pos))!=std::string::npos){src.replace(pos,from.size(),to);pos+=to.size();}};
  replaceAll("var args:array<vec3f,4>;","var arg0=vec3f(0);var arg1=vec3f(0);var arg2=vec3f(0);var arg3=vec3f(0);");
  replaceAll("var aa:array<f32,4>;","var aa0=0.0;var aa1=0.0;var aa2=0.0;var aa3=0.0;");
  for(unsigned i=0;i<4;++i)for(auto suffix:{"", "u"}) {
    replaceAll(fmt::format("args[{}{}]",i,suffix),fmt::format("arg{}",i));
    replaceAll(fmt::format("aa[{}{}]",i,suffix),fmt::format("aa{}",i));
  }
  // Diagnostic ablations deliberately change the image. Each removes one feature
  // from the same interpreter to locate GPU cost; never used for correctness gates.
  if (const char* ablation=std::getenv("PETARI_UBERSHADER_ABLATION")) {
    std::string_view a(ablation);
    auto replaceFunction=[&](std::string_view first,std::string_view next,std::string_view replacement) {
      auto begin=src.find(first);auto end=src.find(next,begin);
      if(begin!=std::string::npos && end!=std::string::npos) src.replace(begin,end-begin,replacement);
    };
    if(a=="fragment") {
      auto begin=src.find("fn run(");auto end=src.find('{',begin)+1;unsigned depth=1;
      while(end<src.size() && depth){if(src[end]=='{')++depth;if(src[end]=='}')--depth;++end;}
      if(!depth)src.replace(begin,end-begin,"fn run(in:VertexOutput)->vec4f {return vec4f(1); }\n");
    }
    if(a=="textures") replaceFunction("fn sample_tex(","fn cop(","fn sample_tex(i:u32,uv:vec2f)->vec4f {return vec4f(1); }\n");
    if(a=="lighting") replaceFunction("fn channel(","@vertex", "fn channel(c:u32,clr:vec4f,pos:vec3f,nrm:vec3f)->vec4f {return clr;}\n");
    std::fprintf(stderr,"[gx uber ablation] feature=%s diagnostic_image=1\n",ablation);
  }
  return src;
}
std::string petari_uber_source(DstAlphaMode mode,uint32_t normal) {return petari_uber_source(mode,normal,16u|(8u<<5));}
wgpu::ShaderModule petari_uber_shader(const ShaderConfig& config,DstAlphaMode mode,uint32_t normal) {
 const auto variant=petari_uber_variant(config);
 static std::mutex mutex;static std::map<std::tuple<unsigned,uint32_t,uint32_t>,wgpu::ShaderModule> modules;
 std::lock_guard lock(mutex);auto key=std::make_tuple(unsigned(mode),normal,variant);auto it=modules.find(key);if(it!=modules.end())return it->second;
 auto start=std::chrono::steady_clock::now();auto src=petari_uber_source(mode,normal,variant);
 PetariPipeline::dumpSource(xxh3_hash_s(src.data(),src.size()),src);
 wgpu::ShaderSourceWGSL wgsl{};wgsl.code=src.c_str();wgpu::ShaderModuleDescriptor desc{.nextInChain=&wgsl,.label="Petari ubershader"};auto module=webgpu::g_device.CreateShaderModule(&desc);
 auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();petariUberShaderMs+=ms;std::fprintf(stderr,"[gx uber shader] mode=%u normal=%u source_bytes=%zu shader_ms=%.3f variant=%u tev=%u texgens=%u\n",unsigned(mode),normal,src.size(),ms,variant,variant&31u,(variant>>5)&15u);
 modules.emplace(key,module);return module;
}
}
