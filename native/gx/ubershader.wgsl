struct Light { pos: vec3f, dir: vec3f, color: vec4f, cos_att: vec3f, dist_att: vec3f, };
struct Uniform {
 cfg: array<vec4u,64>, proj: mat4x4f, postex_mtx: array<mat3x4f,20>, nrm_mtx: array<mat3x4f,10>,
 regs: array<vec4f,4>, lights: array<Light,8>, channels: array<vec4f,8>, konst: array<vec4f,4>,
 postmtx: array<mat3x4f,20>, size_bias: array<vec4f,8>,
};
struct Immediate { vtx_start:u32, current_pnmtx:u32, fog_range_base:u32, _pad:u32, array_start0:vec4u, array_start1:vec4u, array_start2:vec4u, };
var<immediate> imm: Immediate;
@group(0) @binding(0) var<storage,read> vbuf:array<u32>;
@group(0) @binding(1) var<storage,read> abuf:array<u32>;
@group(1) @binding(0) var<uniform> ubuf:Uniform;
fn cfg(i:u32)->u32 {return extractBits(ubuf.cfg[i/16u][(i/4u)%4u],(i%4u)*8u,8u);}
fn array_start(a:u32)->u32 {let i=a-9u;if(i<4u){return imm.array_start0[i];}if(i<8u){return imm.array_start1[i-4u];}return imm.array_start2[i-8u];}
fn scalar(p:ptr<storage,array<u32>>, o:u32,t:u32,f:u32,le:bool)->f32 {
 switch(t) {case 0u:{return fetch_u8_1(p,o,f,le);}case 1u:{return fetch_s8_1(p,o,f,le);}case 2u:{return fetch_u16_1(p,o,f,le);}case 3u:{return fetch_s16_1(p,o,f,le);}default:{return fetch_f32_1(p,o,le);}}
}
fn decode(p:ptr<storage,array<u32>>,o:u32,a:u32,le:bool)->vec4f {
 let b=512u+a*8u;let n=cfg(b+1u);let t=cfg(b+2u);let f=cfg(b+5u);
 if(a==11u || a==12u){switch(t){case 0u:{return fetch_rgb565(p,o,le);}case 1u:{return fetch_rgb8(p,o,le);}case 2u:{return fetch_rgbx8(p,o,le);}case 3u:{return fetch_rgba4(p,o,le);}case 4u:{return fetch_rgba6(p,o,le);}default:{return fetch_rgba8(p,o,le);}}}
 if(a<9u){return vec4f(f32(load_u8(p,o)),0.0,0.0,0.0);}
 var v=vec4f(0.0);var step=1u;if(t==2u || t==3u){step=2u;}if(t==4u){step=4u;}
 for(var i=0u;i<n;i++){v[i]=scalar(p,o+i*step,t,f,le);}return v;
}
fn attr(a:u32,vidx:u32)->vec4f {
 let b=512u+a*8u;let kind=cfg(b);if(kind==0u){if(a==0u){return vec4f(f32(imm.current_pnmtx*3u),0,0,0);}if(a==10u){return vec4f(1,0,0,0);}return vec4f(0);}
 let o=imm.vtx_start+vidx*cfg(821u)+cfg(b+3u);
 if(kind==1u){return decode(&vbuf,o,a,false);}
 var i=load_u8(&vbuf,o);if(kind==3u){i=load_u16(&vbuf,o,false);}
 return decode(&abuf,array_start(a)+i*cfg(b+4u),a,cfg(b+6u)!=0u);
}
struct VertexOutput { @builtin(position) pos:vec4f, @location(0) cc0:vec4f, @location(1) cc1:vec4f,
 @location(2) tc0:vec3f,@location(3) tc1:vec3f,@location(4) tc2:vec3f,@location(5) tc3:vec3f,
 @location(6) tc4:vec3f,@location(7) tc5:vec3f,@location(8) tc6:vec3f,@location(9) tc7:vec3f,@location(10) normal:vec3f, };
fn channel(c:u32,clr:vec4f,pos:vec3f,nrm:vec3f)->vec4f {
 let b=768u+c*8u;var mat=ubuf.channels[c*2u];if(cfg(b)==1u){mat=clr;}if(cfg(b+4u)==0u){return mat;}
 var light=ubuf.channels[c*2u+1u];if(cfg(b+1u)==1u){light=clr;}
 for(var i=0u;i<8u;i++){if((cfg(b+5u)&(1u<<i))==0u){continue;}let l=ubuf.lights[i];var dir=l.pos-pos;let d2=dot(dir,dir);let d=sqrt(d2);dir=dir/d;
 var att=1.0;if(cfg(b+3u)==0u){let spec=select(0.0,max(0.0,dot(nrm,l.dir)),dot(nrm,dir)>=0.0);let v=vec3f(1,spec,spec*spec);var dist_att=l.dist_att;if(cfg(b+2u)!=0u){dist_att=normalize(dist_att);}att=max(0.0,dot(l.cos_att,v)/max(0.0,dot(dist_att,v)));}if(cfg(b+3u)==1u){let cosine=max(0.0,dot(dir,l.dir));att=max(0.0,dot(l.cos_att,vec3f(1,cosine,cosine*cosine))/dot(l.dist_att,vec3f(1,d,d2)));}
 var diff=1.0;if(cfg(b+2u)==1u){diff=dot(dir,nrm);}if(cfg(b+2u)==2u){diff=max(0.0,dot(dir,nrm));}light+=att*diff*l.color;
 }return mat*clamp(light,vec4f(0),vec4f(1));
}
@vertex fn vs_main(@builtin(vertex_index) vidx:u32)->VertexOutput {
 var out:VertexOutput;let p=attr(9u,vidx).xyz;let n=attr(10u,vidx).xyz;let idx=u32(attr(0u,vidx).x)/3u;
 let mv=vec4f(p,1)*ubuf.postex_mtx[idx];let nt=vec4f(n,0)*ubuf.nrm_mtx[idx];let normal=select(nt,normalize(nt),dot(nt,nt)>1e-10);
 out.pos=vec4f(mv,1)*ubuf.proj;out.normal=normal;
 let clr0=attr(11u,vidx);let clr1=attr(12u,vidx);
 out.cc0=vec4f(channel(0u,clr0,mv,normal).rgb,channel(2u,clr0,mv,normal).a);
 out.cc1=vec4f(channel(1u,clr1,mv,normal).rgb,channel(3u,clr1,mv,normal).a);
 var tc:array<vec3f,8>;
 for(var i=0u;i<8u;i++) {if((cfg(824u)&(1u<<i))!=0u){let b=704u+i*8u;let src=cfg(b+1u);var v=vec4f(p,1);if(src==1u){v=vec4f(n,1);}if(src>=4u && src<=11u){v=vec4f(attr(13u+src-4u,vidx).xy,1,1);}
 var m=cfg(b+2u);if(cfg(512u+(1u+i)*8u)!=0u){m=u32(attr(1u+i,vidx).x);}var tmp=v.xyz;if(m!=60u){tmp=v*ubuf.postex_mtx[m/3u];}if(cfg(b)==1u){tmp.z=1.0;}if(cfg(b+4u)!=0u){tmp=normalize(tmp);}let pm=cfg(b+3u);if(pm!=125u){tmp=vec4f(tmp,1)*ubuf.postmtx[(pm-64u)/3u];}tc[i]=tmp;
 }}
 out.tc0=tc[0];out.tc1=tc[1];out.tc2=tc[2];out.tc3=tc[3];out.tc4=tc[4];out.tc5=tc[5];out.tc6=tc[6];out.tc7=tc[7];return out;
}
fn swizzle(v:vec4f,s:u32)->vec4f {let o=804u+s*4u;return vec4f(v[cfg(o)],v[cfg(o+1u)],v[cfg(o+2u)],v[cfg(o+3u)]);}
fn konst(s:u32)->vec4f {if(s<8u){return vec4f(f32(8u-s)/8.0);}if(s<16u){return ubuf.konst[s-12u];}return vec4f(ubuf.konst[(s-16u)%4u][(s-16u)/4u]);}
struct TevRegisters {r0:vec4f,r1:vec4f,r2:vec4f,r3:vec4f,};
fn read_reg(r:TevRegisters,i:u32)->vec4f {switch(i){case 0u:{return r.r0;}case 1u:{return r.r1;}case 2u:{return r.r2;}default:{return r.r3;}}}
fn texcoord(v:VertexOutput,i:u32)->vec3f {switch(i){case 0u:{return v.tc0;}case 1u:{return v.tc1;}case 2u:{return v.tc2;}case 3u:{return v.tc3;}case 4u:{return v.tc4;}case 5u:{return v.tc5;}case 6u:{return v.tc6;}default:{return v.tc7;}}}
fn carg(a:u32,r:TevRegisters,t:vec4f,v:vec4f,k:vec4f)->vec3f {
 if(a<8u){if((a&1u)==0u){return read_reg(r,a/2u).rgb;}return vec3f(read_reg(r,a/2u).a);}
 switch(a){case 8u:{return t.rgb;}case 9u:{return vec3f(t.a);}case 10u:{return v.rgb;}case 11u:{return vec3f(v.a);}case 12u:{return vec3f(1);}case 13u:{return vec3f(0.5);}case 14u:{return k.rgb;}default:{return vec3f(0);}}
}
fn aarg(a:u32,r:TevRegisters,t:vec4f,v:vec4f,k:vec4f)->f32 {if(a<4u){return read_reg(r,a).a;}switch(a){case 4u:{return t.a;}case 5u:{return v.a;}case 6u:{return k.a;}default:{return 0.0;}}}
fn compare(a:u32,b:u32,op:u32)->bool {switch(op){case 0u:{return false;}case 1u:{return a<b;}case 2u:{return a==b;}case 3u:{return a<=b;}case 4u:{return a>b;}case 5u:{return a!=b;}case 6u:{return a>=b;}default:{return true;}}}
