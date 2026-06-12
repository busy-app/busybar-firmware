import{aB as it,an as M,ao as Ge,aN as ye,m as R,aM as nt,aT as $e}from"./entry-f3a1352.js";const le=it(),F={...le,add:t=>{const e=le.toasts.value.find(r=>r.id===t.id);return e?(le.update(t.id,t),e):le.add(t)}},ot=/\{[^{}]+\}/g,st=()=>typeof process=="object"&&Number.parseInt(process?.versions?.node?.substring(0,2))>=18&&process.versions.undici;function at(){return Math.random().toString(36).slice(2,11)}function Ve(t){let{baseUrl:e="",Request:r=globalThis.Request,fetch:i=globalThis.fetch,querySerializer:n,bodySerializer:o,headers:s,requestInitExt:a=void 0,...l}={...t};a=st()?a:void 0,e=Ne(e);const f=[];async function d(u,c){const{baseUrl:p,fetch:y=i,Request:g=r,headers:b,params:T={},parseAs:N="json",querySerializer:h,bodySerializer:E=o??lt,body:q,...V}=c||{};let W=e;p&&(W=Ne(p)??e);let B=typeof n=="function"?n:Se(n);h&&(B=typeof h=="function"?h:Se({...typeof n=="object"?n:{},...h}));const z=q===void 0?void 0:E(q,Oe(s,b,T.header)),ne=Oe(z===void 0||z instanceof FormData?{}:{"Content-Type":"application/json"},s,b,T.header),ee={redirect:"follow",...l,...V,body:z,headers:ne};let A,m,w=new g(ct(u,{baseUrl:W,params:T,querySerializer:B}),ee),_;for(const I in V)I in w||(w[I]=V[I]);if(f.length){A=at(),m=Object.freeze({baseUrl:W,fetch:y,parseAs:N,querySerializer:B,bodySerializer:E});for(const I of f)if(I&&typeof I=="object"&&typeof I.onRequest=="function"){const C=await I.onRequest({request:w,schemaPath:u,params:T,options:m,id:A});if(C)if(C instanceof g)w=C;else if(C instanceof Response){_=C;break}else throw new Error("onRequest: must return new Request() or Response() when modifying the request")}}if(!_){try{_=await y(w,a)}catch(I){let C=I;if(f.length)for(let G=f.length-1;G>=0;G--){const J=f[G];if(J&&typeof J=="object"&&typeof J.onError=="function"){const K=await J.onError({request:w,error:C,schemaPath:u,params:T,options:m,id:A});if(K){if(K instanceof Response){C=void 0,_=K;break}if(K instanceof Error){C=K;continue}throw new Error("onError: must return new Response() or instance of Error")}}}if(C)throw C}if(f.length)for(let I=f.length-1;I>=0;I--){const C=f[I];if(C&&typeof C=="object"&&typeof C.onResponse=="function"){const G=await C.onResponse({request:w,response:_,schemaPath:u,params:T,options:m,id:A});if(G){if(!(G instanceof Response))throw new Error("onResponse: must return new Response() when modifying the response");_=G}}}}if(_.status===204||w.method==="HEAD"||_.headers.get("Content-Length")==="0")return _.ok?{data:void 0,response:_}:{error:void 0,response:_};if(_.ok)return N==="stream"?{data:_.body,response:_}:{data:await _[N](),response:_};let U=await _.text();try{U=JSON.parse(U)}catch{}return{error:U,response:_}}return{request(u,c,p){return d(c,{...p,method:u.toUpperCase()})},GET(u,c){return d(u,{...c,method:"GET"})},PUT(u,c){return d(u,{...c,method:"PUT"})},POST(u,c){return d(u,{...c,method:"POST"})},DELETE(u,c){return d(u,{...c,method:"DELETE"})},OPTIONS(u,c){return d(u,{...c,method:"OPTIONS"})},HEAD(u,c){return d(u,{...c,method:"HEAD"})},PATCH(u,c){return d(u,{...c,method:"PATCH"})},TRACE(u,c){return d(u,{...c,method:"TRACE"})},use(...u){for(const c of u)if(c){if(typeof c!="object"||!("onRequest"in c||"onResponse"in c||"onError"in c))throw new Error("Middleware must be an object with one of `onRequest()`, `onResponse() or `onError()`");f.push(c)}},eject(...u){for(const c of u){const p=f.indexOf(c);p!==-1&&f.splice(p,1)}}}}function ve(t,e,r){if(e==null)return"";if(typeof e=="object")throw new Error("Deeply-nested arrays/objects aren’t supported. Provide your own `querySerializer()` to handle these.");return`${t}=${r?.allowReserved===!0?e:encodeURIComponent(e)}`}function ze(t,e,r){if(!e||typeof e!="object")return"";const i=[],n={simple:",",label:".",matrix:";"}[r.style]||"&";if(r.style!=="deepObject"&&r.explode===!1){for(const a in e)i.push(a,r.allowReserved===!0?e[a]:encodeURIComponent(e[a]));const s=i.join(",");switch(r.style){case"form":return`${t}=${s}`;case"label":return`.${s}`;case"matrix":return`;${t}=${s}`;default:return s}}for(const s in e){const a=r.style==="deepObject"?`${t}[${s}]`:s;i.push(ve(a,e[s],r))}const o=i.join(n);return r.style==="label"||r.style==="matrix"?`${n}${o}`:o}function He(t,e,r){if(!Array.isArray(e))return"";if(r.explode===!1){const o={form:",",spaceDelimited:"%20",pipeDelimited:"|"}[r.style]||",",s=(r.allowReserved===!0?e:e.map(a=>encodeURIComponent(a))).join(o);switch(r.style){case"simple":return s;case"label":return`.${s}`;case"matrix":return`;${t}=${s}`;default:return`${t}=${s}`}}const i={simple:",",label:".",matrix:";"}[r.style]||"&",n=[];for(const o of e)r.style==="simple"||r.style==="label"?n.push(r.allowReserved===!0?o:encodeURIComponent(o)):n.push(ve(t,o,r));return r.style==="label"||r.style==="matrix"?`${i}${n.join(i)}`:n.join(i)}function Se(t){return function(r){const i=[];if(r&&typeof r=="object")for(const n in r){const o=r[n];if(o!=null){if(Array.isArray(o)){if(o.length===0)continue;i.push(He(n,o,{style:"form",explode:!0,...t?.array,allowReserved:t?.allowReserved||!1}));continue}if(typeof o=="object"){i.push(ze(n,o,{style:"deepObject",explode:!0,...t?.object,allowReserved:t?.allowReserved||!1}));continue}i.push(ve(n,o,t))}}return i.join("&")}}function ut(t,e){let r=t;for(const i of t.match(ot)??[]){let n=i.substring(1,i.length-1),o=!1,s="simple";if(n.endsWith("*")&&(o=!0,n=n.substring(0,n.length-1)),n.startsWith(".")?(s="label",n=n.substring(1)):n.startsWith(";")&&(s="matrix",n=n.substring(1)),!e||e[n]===void 0||e[n]===null)continue;const a=e[n];if(Array.isArray(a)){r=r.replace(i,He(n,a,{style:s,explode:o}));continue}if(typeof a=="object"){r=r.replace(i,ze(n,a,{style:s,explode:o}));continue}if(s==="matrix"){r=r.replace(i,`;${ve(n,a)}`);continue}r=r.replace(i,s==="label"?`.${encodeURIComponent(a)}`:encodeURIComponent(a))}return r}function lt(t,e){return t instanceof FormData?t:e&&(e.get instanceof Function?e.get("Content-Type")??e.get("content-type"):e["Content-Type"]??e["content-type"])==="application/x-www-form-urlencoded"?new URLSearchParams(t).toString():JSON.stringify(t)}function ct(t,e){let r=`${e.baseUrl}${t}`;e.params?.path&&(r=ut(r,e.params.path));let i=e.querySerializer(e.params.query??{});return i.startsWith("?")&&(i=i.substring(1)),i&&(r+=`?${i}`),r}function Oe(...t){const e=new Headers;for(const r of t){if(!r||typeof r!="object")continue;const i=r instanceof Headers?r.entries():Object.entries(r);for(const[n,o]of i)if(o===null)e.delete(n);else if(Array.isArray(o))for(const s of o)e.append(n,s);else o!==void 0&&e.set(n,o)}return e}function Ne(t){return t.endsWith("/")?t.substring(0,t.length-1):t}var ft=Object.defineProperty,dt=(t,e,r)=>e in t?ft(t,e,{enumerable:!0,configurable:!0,writable:!0,value:r}):t[e]=r,S=(t,e,r)=>dt(t,typeof e!="symbol"?e+"":e,r);const pt=(t,e)=>{if(typeof FormData<"u"&&t instanceof FormData||typeof Buffer<"u"&&typeof Buffer.isBuffer=="function"&&Buffer.isBuffer(t)||typeof File<"u"&&t instanceof File||typeof Blob<"u"&&t instanceof Blob||typeof ArrayBuffer<"u"&&t instanceof ArrayBuffer||typeof ArrayBuffer<"u"&&ArrayBuffer.isView&&ArrayBuffer.isView(t))return t;let r;return e&&(e instanceof Headers?r=e.get("Content-Type")??e.get("content-type")??void 0:typeof e=="object"&&(r=e["Content-Type"]??e["content-type"]),r==="application/x-www-form-urlencoded")?t&&typeof t=="object"&&!(t instanceof URLSearchParams)?new URLSearchParams(t).toString():String(t):JSON.stringify(t)};async function be(t){const e=(t.headers.get("content-type")||"").includes("application/json")?await t.clone().json():await t.clone().text(),r=typeof e=="object"&&e!==null?e.error||e.message:typeof e=="string"?e:void 0;return Object.assign(new Error(r||`HTTP ${t.status} ${t.statusText}`),{status:t.status,statusText:t.statusText,body:e})}function ht(t,e,r,i=3e3){let n,o=r??void 0,s,a=null;const l=async()=>{n||(a||(a=(async()=>{const u=await e();if(!u.api_semver)throw new Error("Empty API version");n=u.api_semver})().finally(()=>{a=null})),await a)},f={async onRequest({request:u,schemaPath:c}){return o&&u.headers.set("Authorization",`Bearer ${o}`),c!=="/version"&&(await l(),n&&u.headers.set("X-API-Sem-Ver",n),s&&u.headers.set("X-API-Token",s)),u},async onResponse({request:u,response:c,options:p,schemaPath:y}){if(c.ok)return c;if(y==="/version")throw await be(c);if(c.status!==405)throw await be(c);n=void 0,await l(),n&&u.headers.set("X-API-Sem-Ver",n),o&&u.headers.set("Authorization",`Bearer ${o}`);const g=await(p.fetch??fetch)(u);if(g.ok)return g;throw await be(g)}},d=Ve({baseUrl:t,bodySerializer:pt});return d.withTimeout=async(u,c=i)=>{if(c<=0)return await u();const p=new AbortController,y=setTimeout(()=>p.abort(),c);try{return await u(p.signal)}catch(g){throw g instanceof DOMException&&g.name==="AbortError"?new Error(`Request timed out after ${c}ms`):g}finally{clearTimeout(y)}},d.use(f),{client:d,setApiKey:u=>{s=u},setToken:u=>{o=u}}}async function mt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/version",{signal:n}),e?.timeout);if(i)throw i;return r}async function yt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function vt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/system",{signal:n}),e?.timeout);if(i)throw i;return r}async function gt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/power",{signal:n}),e?.timeout);if(i)throw i;return r}async function bt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/device",{signal:n}),e?.timeout);if(i)throw i;return r}async function wt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/firmware",{signal:n}),e?.timeout);if(i)throw i;return r}async function Et(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/transport",{signal:n}),e?.timeout);if(i)throw i;return r}class At{async SystemVersionGet(e){const r=await mt(this.apiClient,e);return this.apiSemver=r.api_semver,r}async SystemStatusGet(e){return await yt(this.apiClient,e)}async SystemInfoGet(e){return await vt(this.apiClient,e)}async SystemStatusPowerGet(e){return await gt(this.apiClient,e)}async SystemStatusDeviceGet(e){return await bt(this.apiClient,e)}async SystemStatusFirmwareGet(e){return await wt(this.apiClient,e)}async SystemTransportGet(e){return await Et(this.apiClient,e)}}async function _t(t,e){const{file:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/update",{headers:{"Content-Type":"application/octet-stream"},body:r,signal:o}),e.timeout);if(n)throw n;return i}async function Tt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/update/check",{signal:n}),e?.timeout);if(i)throw i;return r}async function Rt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/update/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function St(t,e){const{version:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.GET("/update/changelog",{params:{query:{version:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Ot(t,e){const{version:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/update/install",{params:{query:{version:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Nt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/update/abort_download",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ct(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/update/autoupdate",{signal:n}),e?.timeout);if(i)throw i;return r}async function kt(t,e){const{is_enabled:r,interval_start:i,interval_end:n}=e,{data:o,error:s}=await t.withTimeout(a=>t.POST("/update/autoupdate",{body:{is_enabled:r,interval_start:i,interval_end:n},signal:a}),e.timeout);if(s)throw s;return o}class It{async UpdateFromFile(e){return await _t(this.apiClient,e)}async UpdateCheck(e){return await Tt(this.apiClient,e)}async UpdateStatusGet(e){return await Rt(this.apiClient,e)}async UpdateChangelogGet(e){return await St(this.apiClient,e)}async UpdateInstall(e){return await Ot(this.apiClient,e)}async UpdateAbort(e){return await Nt(this.apiClient,e)}async UpdateAutoUpdateGet(e){return await Ct(this.apiClient,e)}async UpdateAutoUpdateSet(e){return await kt(this.apiClient,e)}}async function qt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/time",{signal:n}),e?.timeout);if(i)throw i;return r}async function Pt(t,e){const{timestamp:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/time/timestamp",{params:{query:{timestamp:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Dt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/time/timezone",{signal:n}),e?.timeout);if(i)throw i;return r}async function xt(t,e){const{timezone:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/time/timezone",{params:{query:{timezone:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Ut(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/time/tzlist",{signal:n}),e?.timeout);if(i)throw i;return r}class Lt{async TimeGet(e){return await qt(this.apiClient,e)}async TimeTimestampSet(e){return await Pt(this.apiClient,e)}async TimeTimezoneGet(e){return await Dt(this.apiClient,e)}async TimeTimezoneSet(e){return await xt(this.apiClient,e)}async TimeTzListGet(e){return await Ut(this.apiClient,e)}}async function jt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/account/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function Bt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/account/info",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ft(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/account/profile",{signal:n}),e?.timeout);if(i)throw i;return r}async function Mt(t,e){const{profile:r,custom_url:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/account/profile",{params:{query:{profile:r,custom_url:i}},signal:s}),e.timeout);if(o)throw o;return n}async function Wt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/account",{signal:n}),e?.timeout);if(i)throw i;return r}async function Gt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/account/link",{signal:n}),e?.timeout);if(i)throw i;return r}class $t{async AccountInfoGet(e){return await Bt(this.apiClient,e)}async AccountStateGet(e){return await jt(this.apiClient,e)}async AccountProfileGet(e){return await Ft(this.apiClient,e)}async AccountProfileSet(e){return await Mt(this.apiClient,e)}async AccountUnlink(e){return await Wt(this.apiClient,e)}async AccountLink(e){return await Gt(this.apiClient,e)}}async function Vt(t,e){const{application_name:r,elements:i,priority:n=50,timeout:o}=e,{data:s,error:a}=await t.withTimeout(l=>t.POST("/display/draw",{body:{application_name:r,priority:n,elements:i},signal:l}),o);if(a)throw a;return s}async function zt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/display/draw",{params:{query:{application_name:e?.application_name}},signal:n}),e?.timeout);if(i)throw i;return r}async function Ht(t,e){const{display:r,timeout:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.GET("/screen",{params:{query:{display:r}},parseAs:"blob",signal:s}),i);if(o)throw o;return n}async function Jt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/display/brightness",{signal:n}),e?.timeout);if(i)throw i;return r}async function Kt(t,e){const{value:r}=e,i=(s=>{if(typeof s=="number"){if(s<0||s>100)throw new Error("Brightness value must be between 0 and 100 or 'auto'");return String(s)}return"auto"})(r),{data:n,error:o}=await t.withTimeout(s=>t.POST("/display/brightness",{params:{query:{value:i}},signal:s}),e.timeout);if(o)throw o;return n}class Yt{async DisplayDraw(e){return await Vt(this.apiClient,e)}async DisplayClear(e){return await zt(this.apiClient,e)}async DisplayScreenFrameGet(e){return await Ht(this.apiClient,e)}async DisplayBrightnessGet(e){return await Jt(this.apiClient,e)}async DisplayBrightnessSet(e){return await Kt(this.apiClient,e)}}async function Xt(t,e){const{application_name:r,path:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/audio/play",{params:{query:{application_name:r,path:i}},signal:s}),e.timeout);if(o)throw o;return n}async function Zt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/audio/play",{signal:n}),e?.timeout);if(i)throw i;return r}async function Qt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/audio/volume",{signal:n}),e?.timeout);if(i)throw i;return r}async function er(t,e){const{volume:r,silent:i}=e;if(typeof r!="number"||r<0||r>100)throw new Error("Volume must be a number between 0 and 100");const{data:n,error:o}=await t.withTimeout(s=>t.POST("/audio/volume",{params:{query:{volume:r,silent:i}},signal:s}),e.timeout);if(o)throw o;return n}class tr{async AudioPlay(e){return await Xt(this.apiClient,e)}async AudioStop(e){return await Zt(this.apiClient,e)}async AudioVolumeGet(e){return await Qt(this.apiClient,e)}async AudioVolumeSet(e){return await er(this.apiClient,e)}}async function rr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/wifi/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function ir(t,e){const{ssid:r,password:i,security:n,ip_config:o}=e,{data:s,error:a}=await t.withTimeout(l=>t.POST("/wifi/connect",{body:{ssid:r,password:i,security:n,ip_config:o},signal:l}),e.timeout);if(a)throw a;return s}async function nr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/wifi/disconnect",{signal:n}),e?.timeout);if(i)throw i;return r}async function or(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/wifi/networks",{signal:n}),e?.timeout);if(i)throw i;return r}class sr{async WifiStatusGet(e){return await rr(this.apiClient,e)}async WifiConnect(e){return await ir(this.apiClient,e)}async WifiDisconnect(e){return await nr(this.apiClient,e)}async WifiNetworksGet(e){return await or(this.apiClient,e)}}async function ar(t,e){const{path:r,file:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/storage/write",{params:{query:{path:r}},headers:{"Content-Type":"application/octet-stream"},body:i,signal:s}),e.timeout);if(o)throw o;return n}async function ur(t,e){const{path:r,as_array_buffer:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.GET("/storage/read",{params:{query:{path:r}},parseAs:i?"arrayBuffer":"blob",signal:s}),e.timeout);if(o)throw o;return n}async function lr(t,e){const{path:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.GET("/storage/list",{params:{query:{path:r}},signal:o}),e.timeout);if(n)throw n;return i}async function cr(t,e){const{path:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.DELETE("/storage/remove",{params:{query:{path:r}},signal:o}),e.timeout);if(n)throw n;return i}async function fr(t,e){const{path:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/storage/mkdir",{params:{query:{path:r}},signal:o}),e.timeout);if(n)throw n;return i}async function dr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/storage/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function pr(t,e){const{path:r,new_path:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/storage/rename",{params:{query:{path:r,new_path:i}},signal:s}),e.timeout);if(o)throw o;return n}class hr{async StorageWrite(e){return await ar(this.apiClient,e)}async StorageRead(e){return await ur(this.apiClient,e)}async StorageListGet(e){return await lr(this.apiClient,e)}async StorageRemove(e){return await cr(this.apiClient,e)}async StorageMkdir(e){return await fr(this.apiClient,e)}async StorageStatusGet(e){return await dr(this.apiClient,e)}async StorageRename(e){return await pr(this.apiClient,e)}}async function mr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/access",{signal:n}),e?.timeout);if(i)throw i;return r}async function yr(t,e){const{mode:r,key:i}=e,n=i??"";if(String(n).trim()&&!/^\d{4,10}$/.test(String(n)))throw new Error("Key must be a string of 4 to 10 digits");const{data:o,error:s}=await t.withTimeout(a=>t.POST("/access",{params:{query:{mode:r,key:n}},signal:a}),e.timeout);if(s)throw s;return o}async function vr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/name",{signal:n}),e?.timeout);if(i)throw i;return r}async function gr(t,e){const{name:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/name",{body:{name:r},signal:o}),e.timeout);if(n)throw n;return i}class br{async SettingsAccessGet(e){return await mr(this.apiClient,e)}async SettingsAccessSet(e){const r=await yr(this.apiClient,e);return e.mode==="key"&&e.key&&this.setApiKey(e.key),r}async SettingsNameGet(e){return await vr(this.apiClient,e)}async SettingsNameSet(e){return await gr(this.apiClient,e)}}async function wr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/ble/enable",{signal:n}),e?.timeout);if(i)throw i;return r}async function Er(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/ble/disable",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ar(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/ble/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function _r(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/ble/status",{signal:n}),e?.timeout);if(i)throw i;return r}class Tr{async BleEnable(e){return await wr(this.apiClient,e)}async BleDisable(e){return await Er(this.apiClient,e)}async BleUnpair(e){return await Ar(this.apiClient,e)}async BleStatusGet(e){return await _r(this.apiClient,e)}}async function Rr(t,e){const{key:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/input",{params:{query:{key:r}},signal:o}),e.timeout);if(n)throw n;return i}class Sr{async InputSend(e){return await Rr(this.apiClient,e)}}async function Or(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/smart_home/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function Nr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/smart_home/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function Cr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/smart_home/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function kr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/smart_home/switch",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ir(t,e){const{timeout:r,...i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/smart_home/switch",{body:i,signal:s}),r);if(o)throw o;return n}class qr{async SmartHomePairingGet(e){return await Or(this.apiClient,e)}async SmartHomePair(e){return await Nr(this.apiClient,e)}async SmartHomeErase(e){return await Cr(this.apiClient,e)}async SmartHomeSwitchStateGet(e){return await kr(this.apiClient,e)}async SmartHomeSwitchStateSet(e){return await Ir(this.apiClient,e)}async MatterStatusGet(e){return await this.SmartHomePairingGet(e)}async MatterPair(e){return await this.SmartHomePair(e)}async MatterErase(e){return await this.SmartHomeErase(e)}}async function Pr(t,e){const{application_name:r,file:i,data:n}=e,{data:o,error:s}=await t.withTimeout(a=>t.POST("/assets/upload",{params:{query:{application_name:r,file:i}},headers:{"Content-Type":"application/octet-stream"},body:n,signal:a}),e.timeout);if(s)throw s;return o}async function Dr(t,e){const{application_name:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.DELETE("/assets/upload",{params:{query:{application_name:r}},signal:o}),e.timeout);if(n)throw n;return i}class xr{async AssetsUpload(e){return await Pr(this.apiClient,e)}async AssetsDelete(e){return await Dr(this.apiClient,e)}}const Ur="http://10.0.4.20",Lr="https://proxy.busy.app",jr=/^https?:\/\/proxy(?:\.(?:dev|test|stage))?\.busy\.app$/i;function Br(t){const e=t.split(".");if(e.length!==4)return!1;for(const r of e){if(r.length===0||r.length>1&&r[0]==="0"||!/^\d+$/.test(r))return!1;const i=Number(r);if(i<0||i>255)return!1}return!0}function Fr(t){return/\.local$/i.test(t)}class Je{constructor(e){if(S(this,"addr"),S(this,"apiSemver"),S(this,"apiClient"),S(this,"setApiKeyFn"),S(this,"setTokenFn"),S(this,"connectionType","unknown"),!e||!e.addr&&!e.token)this.addr=Ur;else if(!e.addr)this.addr=Lr;else{let o=e.addr.trim();if(/^https?:\/\//i.test(o)||(o=`http://${o}`),jr.test(o)&&!e.token)throw new Error("Token is required. Please provide it.");this.addr=o}this.apiSemver="";const{client:r,setApiKey:i,setToken:n}=ht(`${this.addr}/api/`,this.SystemVersionGet.bind(this),e?.token,e?.timeout);this.apiClient=r,this.setApiKeyFn=i,this.setTokenFn=n,this.detectConnectionType()}async detectConnectionType(){const e=new URL(this.addr).hostname;if(!Br(e)&&!Fr(e)){this.connectionType="wifi";return}const r=Ve({baseUrl:`${this.addr}/api/`});try{const{response:i}=await r.GET("/name");if(i.status===401||i.status===403)this.connectionType="wifi";else if(i.ok)this.connectionType="usb";else throw new Error(`Failed to detect connection type. Status: ${i.status}`)}catch(i){throw i}}setApiKey(e){this.setApiKeyFn(e)}setToken(e){this.setTokenFn(e)}}function Mr(t,e){e.forEach(r=>{Object.getOwnPropertyNames(r.prototype).forEach(i=>{Object.defineProperty(t.prototype,i,Object.getOwnPropertyDescriptor(r.prototype,i)||Object.create(null))})})}Mr(Je,[At,It,Lt,$t,Yt,tr,sr,hr,br,Tr,Sr,qr,xr]);var L=(t=>(t.CONNECTION_FAILED="CONNECTION_FAILED",t.RECONNECT_FAILED="RECONNECT_FAILED",t.CONNECTION_LOST="CONNECTION_LOST",t.CONNECTION_TIMEOUT="CONNECTION_TIMEOUT",t.AUTH_FAILED="AUTH_FAILED",t.AUTH_REFRESH_FAILED="AUTH_REFRESH_FAILED",t.DEVICE_ERROR="DEVICE_ERROR",t.DECODE_ERROR="DECODE_ERROR",t.FRAME_PROCESS_ERROR="FRAME_PROCESS_ERROR",t.STREAM_ALREADY_STARTED="STREAM_ALREADY_STARTED",t.WORKER_INIT_FAILED="WORKER_INIT_FAILED",t.UNKNOWN_ERROR="UNKNOWN_ERROR",t))(L||{});class te extends Error{constructor(e,r,i){super(r),this.code=e,this.data=i,this.name="StateStreamError"}}var D=(t=>(t.IDLE="IDLE",t.STARTING="STARTING",t.RUNNING="RUNNING",t.STOPPED="STOPPED",t.FAILED="FAILED",t))(D||{}),Y=(t=>(t.DISCONNECTED="DISCONNECTED",t.CONNECTING="CONNECTING",t.CONNECTED="CONNECTED",t.RECONNECTING="RECONNECTING",t))(Y||{}),re=(t=>(t.UNAUTHENTICATED="UNAUTHENTICATED",t.AUTHENTICATING="AUTHENTICATING",t.AUTHENTICATED="AUTHENTICATED",t.REAUTHENTICATING="REAUTHENTICATING",t.FAILED="FAILED",t))(re||{}),j=(t=>(t.NONE="NONE",t.ACTIVE="ACTIVE",t.STALE="STALE",t))(j||{}),ae=(t=>(t.OFF="OFF",t.INITIALIZING="INITIALIZING",t.READY="READY",t.ERROR="ERROR",t))(ae||{});const Wr=5,Gr=5,$r=500,Ke=`var commonjsGlobal = typeof globalThis < "u" ? globalThis : typeof window < "u" ? window : typeof global < "u" ? global : typeof self < "u" ? self : {}, src = { exports: {} }, indexLight = { exports: {} }, indexMinimal = {}, minimal = {}, aspromise, hasRequiredAspromise;
function requireAspromise() {
  if (hasRequiredAspromise) return aspromise;
  hasRequiredAspromise = 1, aspromise = a;
  function a(f, h) {
    for (var c = new Array(arguments.length - 1), d = 0, n = 2, e = !0; n < arguments.length; )
      c[d++] = arguments[n++];
    return new Promise(function(i, t) {
      c[d] = function(s) {
        if (e)
          if (e = !1, s)
            t(s);
          else {
            for (var u = new Array(arguments.length - 1), o = 0; o < u.length; )
              u[o++] = arguments[o];
            i.apply(null, u);
          }
      };
      try {
        f.apply(h || null, c);
      } catch (l) {
        e && (e = !1, t(l));
      }
    });
  }
  return aspromise;
}
var base64 = {}, hasRequiredBase64;
function requireBase64() {
  return hasRequiredBase64 || (hasRequiredBase64 = 1, function(a) {
    var f = a;
    f.length = function(r) {
      var i = r.length;
      if (!i)
        return 0;
      for (var t = 0; --i % 4 > 1 && r.charAt(i) === "="; )
        ++t;
      return Math.ceil(r.length * 3) / 4 - t;
    };
    for (var h = new Array(64), c = new Array(123), d = 0; d < 64; )
      c[h[d] = d < 26 ? d + 65 : d < 52 ? d + 71 : d < 62 ? d - 4 : d - 59 | 43] = d++;
    f.encode = function(r, i, t) {
      for (var l = null, s = [], u = 0, o = 0, p; i < t; ) {
        var y = r[i++];
        switch (o) {
          case 0:
            s[u++] = h[y >> 2], p = (y & 3) << 4, o = 1;
            break;
          case 1:
            s[u++] = h[p | y >> 4], p = (y & 15) << 2, o = 2;
            break;
          case 2:
            s[u++] = h[p | y >> 6], s[u++] = h[y & 63], o = 0;
            break;
        }
        u > 8191 && ((l || (l = [])).push(String.fromCharCode.apply(String, s)), u = 0);
      }
      return o && (s[u++] = h[p], s[u++] = 61, o === 1 && (s[u++] = 61)), l ? (u && l.push(String.fromCharCode.apply(String, s.slice(0, u))), l.join("")) : String.fromCharCode.apply(String, s.slice(0, u));
    };
    var n = "invalid encoding";
    f.decode = function(r, i, t) {
      for (var l = t, s = 0, u, o = 0; o < r.length; ) {
        var p = r.charCodeAt(o++);
        if (p === 61 && s > 1)
          break;
        if ((p = c[p]) === void 0)
          throw Error(n);
        switch (s) {
          case 0:
            u = p, s = 1;
            break;
          case 1:
            i[t++] = u << 2 | (p & 48) >> 4, u = p, s = 2;
            break;
          case 2:
            i[t++] = (u & 15) << 4 | (p & 60) >> 2, u = p, s = 3;
            break;
          case 3:
            i[t++] = (u & 3) << 6 | p, s = 0;
            break;
        }
      }
      if (s === 1)
        throw Error(n);
      return t - l;
    }, f.test = function(r) {
      return /^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$/.test(r);
    };
  }(base64)), base64;
}
var eventemitter, hasRequiredEventemitter;
function requireEventemitter() {
  if (hasRequiredEventemitter) return eventemitter;
  hasRequiredEventemitter = 1, eventemitter = a;
  function a() {
    this._listeners = {};
  }
  return a.prototype.on = function(h, c, d) {
    return (this._listeners[h] || (this._listeners[h] = [])).push({
      fn: c,
      ctx: d || this
    }), this;
  }, a.prototype.off = function(h, c) {
    if (h === void 0)
      this._listeners = {};
    else if (c === void 0)
      this._listeners[h] = [];
    else
      for (var d = this._listeners[h], n = 0; n < d.length; )
        d[n].fn === c ? d.splice(n, 1) : ++n;
    return this;
  }, a.prototype.emit = function(h) {
    var c = this._listeners[h];
    if (c) {
      for (var d = [], n = 1; n < arguments.length; )
        d.push(arguments[n++]);
      for (n = 0; n < c.length; )
        c[n].fn.apply(c[n++].ctx, d);
    }
    return this;
  }, eventemitter;
}
var float, hasRequiredFloat;
function requireFloat() {
  if (hasRequiredFloat) return float;
  hasRequiredFloat = 1, float = a(a);
  function a(n) {
    return typeof Float32Array < "u" ? function() {
      var e = new Float32Array([-0]), r = new Uint8Array(e.buffer), i = r[3] === 128;
      function t(o, p, y) {
        e[0] = o, p[y] = r[0], p[y + 1] = r[1], p[y + 2] = r[2], p[y + 3] = r[3];
      }
      function l(o, p, y) {
        e[0] = o, p[y] = r[3], p[y + 1] = r[2], p[y + 2] = r[1], p[y + 3] = r[0];
      }
      n.writeFloatLE = i ? t : l, n.writeFloatBE = i ? l : t;
      function s(o, p) {
        return r[0] = o[p], r[1] = o[p + 1], r[2] = o[p + 2], r[3] = o[p + 3], e[0];
      }
      function u(o, p) {
        return r[3] = o[p], r[2] = o[p + 1], r[1] = o[p + 2], r[0] = o[p + 3], e[0];
      }
      n.readFloatLE = i ? s : u, n.readFloatBE = i ? u : s;
    }() : function() {
      function e(i, t, l, s) {
        var u = t < 0 ? 1 : 0;
        if (u && (t = -t), t === 0)
          i(1 / t > 0 ? (
            /* positive */
            0
          ) : (
            /* negative 0 */
            2147483648
          ), l, s);
        else if (isNaN(t))
          i(2143289344, l, s);
        else if (t > 34028234663852886e22)
          i((u << 31 | 2139095040) >>> 0, l, s);
        else if (t < 11754943508222875e-54)
          i((u << 31 | Math.round(t / 1401298464324817e-60)) >>> 0, l, s);
        else {
          var o = Math.floor(Math.log(t) / Math.LN2), p = Math.round(t * Math.pow(2, -o) * 8388608) & 8388607;
          i((u << 31 | o + 127 << 23 | p) >>> 0, l, s);
        }
      }
      n.writeFloatLE = e.bind(null, f), n.writeFloatBE = e.bind(null, h);
      function r(i, t, l) {
        var s = i(t, l), u = (s >> 31) * 2 + 1, o = s >>> 23 & 255, p = s & 8388607;
        return o === 255 ? p ? NaN : u * (1 / 0) : o === 0 ? u * 1401298464324817e-60 * p : u * Math.pow(2, o - 150) * (p + 8388608);
      }
      n.readFloatLE = r.bind(null, c), n.readFloatBE = r.bind(null, d);
    }(), typeof Float64Array < "u" ? function() {
      var e = new Float64Array([-0]), r = new Uint8Array(e.buffer), i = r[7] === 128;
      function t(o, p, y) {
        e[0] = o, p[y] = r[0], p[y + 1] = r[1], p[y + 2] = r[2], p[y + 3] = r[3], p[y + 4] = r[4], p[y + 5] = r[5], p[y + 6] = r[6], p[y + 7] = r[7];
      }
      function l(o, p, y) {
        e[0] = o, p[y] = r[7], p[y + 1] = r[6], p[y + 2] = r[5], p[y + 3] = r[4], p[y + 4] = r[3], p[y + 5] = r[2], p[y + 6] = r[1], p[y + 7] = r[0];
      }
      n.writeDoubleLE = i ? t : l, n.writeDoubleBE = i ? l : t;
      function s(o, p) {
        return r[0] = o[p], r[1] = o[p + 1], r[2] = o[p + 2], r[3] = o[p + 3], r[4] = o[p + 4], r[5] = o[p + 5], r[6] = o[p + 6], r[7] = o[p + 7], e[0];
      }
      function u(o, p) {
        return r[7] = o[p], r[6] = o[p + 1], r[5] = o[p + 2], r[4] = o[p + 3], r[3] = o[p + 4], r[2] = o[p + 5], r[1] = o[p + 6], r[0] = o[p + 7], e[0];
      }
      n.readDoubleLE = i ? s : u, n.readDoubleBE = i ? u : s;
    }() : function() {
      function e(i, t, l, s, u, o) {
        var p = s < 0 ? 1 : 0;
        if (p && (s = -s), s === 0)
          i(0, u, o + t), i(1 / s > 0 ? (
            /* positive */
            0
          ) : (
            /* negative 0 */
            2147483648
          ), u, o + l);
        else if (isNaN(s))
          i(0, u, o + t), i(2146959360, u, o + l);
        else if (s > 17976931348623157e292)
          i(0, u, o + t), i((p << 31 | 2146435072) >>> 0, u, o + l);
        else {
          var y;
          if (s < 22250738585072014e-324)
            y = s / 5e-324, i(y >>> 0, u, o + t), i((p << 31 | y / 4294967296) >>> 0, u, o + l);
          else {
            var E = Math.floor(Math.log(s) / Math.LN2);
            E === 1024 && (E = 1023), y = s * Math.pow(2, -E), i(y * 4503599627370496 >>> 0, u, o + t), i((p << 31 | E + 1023 << 20 | y * 1048576 & 1048575) >>> 0, u, o + l);
          }
        }
      }
      n.writeDoubleLE = e.bind(null, f, 0, 4), n.writeDoubleBE = e.bind(null, h, 4, 0);
      function r(i, t, l, s, u) {
        var o = i(s, u + t), p = i(s, u + l), y = (p >> 31) * 2 + 1, E = p >>> 20 & 2047, v = 4294967296 * (p & 1048575) + o;
        return E === 2047 ? v ? NaN : y * (1 / 0) : E === 0 ? y * 5e-324 * v : y * Math.pow(2, E - 1075) * (v + 4503599627370496);
      }
      n.readDoubleLE = r.bind(null, c, 0, 4), n.readDoubleBE = r.bind(null, d, 4, 0);
    }(), n;
  }
  function f(n, e, r) {
    e[r] = n & 255, e[r + 1] = n >>> 8 & 255, e[r + 2] = n >>> 16 & 255, e[r + 3] = n >>> 24;
  }
  function h(n, e, r) {
    e[r] = n >>> 24, e[r + 1] = n >>> 16 & 255, e[r + 2] = n >>> 8 & 255, e[r + 3] = n & 255;
  }
  function c(n, e) {
    return (n[e] | n[e + 1] << 8 | n[e + 2] << 16 | n[e + 3] << 24) >>> 0;
  }
  function d(n, e) {
    return (n[e] << 24 | n[e + 1] << 16 | n[e + 2] << 8 | n[e + 3]) >>> 0;
  }
  return float;
}
var inquire_1, hasRequiredInquire;
function requireInquire() {
  if (hasRequiredInquire) return inquire_1;
  hasRequiredInquire = 1, inquire_1 = inquire;
  function inquire(moduleName) {
    try {
      var mod = eval("quire".replace(/^/, "re"))(moduleName);
      if (mod && (mod.length || Object.keys(mod).length))
        return mod;
    } catch (a) {
    }
    return null;
  }
  return inquire_1;
}
var utf8 = {}, hasRequiredUtf8;
function requireUtf8() {
  return hasRequiredUtf8 || (hasRequiredUtf8 = 1, function(a) {
    var f = a;
    f.length = function(c) {
      for (var d = 0, n = 0, e = 0; e < c.length; ++e)
        n = c.charCodeAt(e), n < 128 ? d += 1 : n < 2048 ? d += 2 : (n & 64512) === 55296 && (c.charCodeAt(e + 1) & 64512) === 56320 ? (++e, d += 4) : d += 3;
      return d;
    }, f.read = function(c, d, n) {
      var e = n - d;
      if (e < 1)
        return "";
      for (var r = null, i = [], t = 0, l; d < n; )
        l = c[d++], l < 128 ? i[t++] = l : l > 191 && l < 224 ? i[t++] = (l & 31) << 6 | c[d++] & 63 : l > 239 && l < 365 ? (l = ((l & 7) << 18 | (c[d++] & 63) << 12 | (c[d++] & 63) << 6 | c[d++] & 63) - 65536, i[t++] = 55296 + (l >> 10), i[t++] = 56320 + (l & 1023)) : i[t++] = (l & 15) << 12 | (c[d++] & 63) << 6 | c[d++] & 63, t > 8191 && ((r || (r = [])).push(String.fromCharCode.apply(String, i)), t = 0);
      return r ? (t && r.push(String.fromCharCode.apply(String, i.slice(0, t))), r.join("")) : String.fromCharCode.apply(String, i.slice(0, t));
    }, f.write = function(c, d, n) {
      for (var e = n, r, i, t = 0; t < c.length; ++t)
        r = c.charCodeAt(t), r < 128 ? d[n++] = r : r < 2048 ? (d[n++] = r >> 6 | 192, d[n++] = r & 63 | 128) : (r & 64512) === 55296 && ((i = c.charCodeAt(t + 1)) & 64512) === 56320 ? (r = 65536 + ((r & 1023) << 10) + (i & 1023), ++t, d[n++] = r >> 18 | 240, d[n++] = r >> 12 & 63 | 128, d[n++] = r >> 6 & 63 | 128, d[n++] = r & 63 | 128) : (d[n++] = r >> 12 | 224, d[n++] = r >> 6 & 63 | 128, d[n++] = r & 63 | 128);
      return n - e;
    };
  }(utf8)), utf8;
}
var pool_1, hasRequiredPool;
function requirePool() {
  if (hasRequiredPool) return pool_1;
  hasRequiredPool = 1, pool_1 = a;
  function a(f, h, c) {
    var d = c || 8192, n = d >>> 1, e = null, r = d;
    return function(t) {
      if (t < 1 || t > n)
        return f(t);
      r + t > d && (e = f(d), r = 0);
      var l = h.call(e, r, r += t);
      return r & 7 && (r = (r | 7) + 1), l;
    };
  }
  return pool_1;
}
var longbits, hasRequiredLongbits;
function requireLongbits() {
  if (hasRequiredLongbits) return longbits;
  hasRequiredLongbits = 1, longbits = f;
  var a = requireMinimal();
  function f(n, e) {
    this.lo = n >>> 0, this.hi = e >>> 0;
  }
  var h = f.zero = new f(0, 0);
  h.toNumber = function() {
    return 0;
  }, h.zzEncode = h.zzDecode = function() {
    return this;
  }, h.length = function() {
    return 1;
  };
  var c = f.zeroHash = "\\0\\0\\0\\0\\0\\0\\0\\0";
  f.fromNumber = function(e) {
    if (e === 0)
      return h;
    var r = e < 0;
    r && (e = -e);
    var i = e >>> 0, t = (e - i) / 4294967296 >>> 0;
    return r && (t = ~t >>> 0, i = ~i >>> 0, ++i > 4294967295 && (i = 0, ++t > 4294967295 && (t = 0))), new f(i, t);
  }, f.from = function(e) {
    if (typeof e == "number")
      return f.fromNumber(e);
    if (a.isString(e))
      if (a.Long)
        e = a.Long.fromString(e);
      else
        return f.fromNumber(parseInt(e, 10));
    return e.low || e.high ? new f(e.low >>> 0, e.high >>> 0) : h;
  }, f.prototype.toNumber = function(e) {
    if (!e && this.hi >>> 31) {
      var r = ~this.lo + 1 >>> 0, i = ~this.hi >>> 0;
      return r || (i = i + 1 >>> 0), -(r + i * 4294967296);
    }
    return this.lo + this.hi * 4294967296;
  }, f.prototype.toLong = function(e) {
    return a.Long ? new a.Long(this.lo | 0, this.hi | 0, !!e) : { low: this.lo | 0, high: this.hi | 0, unsigned: !!e };
  };
  var d = String.prototype.charCodeAt;
  return f.fromHash = function(e) {
    return e === c ? h : new f(
      (d.call(e, 0) | d.call(e, 1) << 8 | d.call(e, 2) << 16 | d.call(e, 3) << 24) >>> 0,
      (d.call(e, 4) | d.call(e, 5) << 8 | d.call(e, 6) << 16 | d.call(e, 7) << 24) >>> 0
    );
  }, f.prototype.toHash = function() {
    return String.fromCharCode(
      this.lo & 255,
      this.lo >>> 8 & 255,
      this.lo >>> 16 & 255,
      this.lo >>> 24,
      this.hi & 255,
      this.hi >>> 8 & 255,
      this.hi >>> 16 & 255,
      this.hi >>> 24
    );
  }, f.prototype.zzEncode = function() {
    var e = this.hi >> 31;
    return this.hi = ((this.hi << 1 | this.lo >>> 31) ^ e) >>> 0, this.lo = (this.lo << 1 ^ e) >>> 0, this;
  }, f.prototype.zzDecode = function() {
    var e = -(this.lo & 1);
    return this.lo = ((this.lo >>> 1 | this.hi << 31) ^ e) >>> 0, this.hi = (this.hi >>> 1 ^ e) >>> 0, this;
  }, f.prototype.length = function() {
    var e = this.lo, r = (this.lo >>> 28 | this.hi << 4) >>> 0, i = this.hi >>> 24;
    return i === 0 ? r === 0 ? e < 16384 ? e < 128 ? 1 : 2 : e < 2097152 ? 3 : 4 : r < 16384 ? r < 128 ? 5 : 6 : r < 2097152 ? 7 : 8 : i < 128 ? 9 : 10;
  }, longbits;
}
var hasRequiredMinimal;
function requireMinimal() {
  return hasRequiredMinimal || (hasRequiredMinimal = 1, function(a) {
    var f = a;
    f.asPromise = requireAspromise(), f.base64 = requireBase64(), f.EventEmitter = requireEventemitter(), f.float = requireFloat(), f.inquire = requireInquire(), f.utf8 = requireUtf8(), f.pool = requirePool(), f.LongBits = requireLongbits(), f.isNode = !!(typeof commonjsGlobal < "u" && commonjsGlobal && commonjsGlobal.process && commonjsGlobal.process.versions && commonjsGlobal.process.versions.node), f.global = f.isNode && commonjsGlobal || typeof window < "u" && window || typeof self < "u" && self || minimal, f.emptyArray = Object.freeze ? Object.freeze([]) : (
      /* istanbul ignore next */
      []
    ), f.emptyObject = Object.freeze ? Object.freeze({}) : (
      /* istanbul ignore next */
      {}
    ), f.isInteger = Number.isInteger || /* istanbul ignore next */
    function(n) {
      return typeof n == "number" && isFinite(n) && Math.floor(n) === n;
    }, f.isString = function(n) {
      return typeof n == "string" || n instanceof String;
    }, f.isObject = function(n) {
      return n && typeof n == "object";
    }, f.isset = /**
     * Checks if a property on a message is considered to be present.
     * @param {Object} obj Plain object or message instance
     * @param {string} prop Property name
     * @returns {boolean} \`true\` if considered to be present, otherwise \`false\`
     */
    f.isSet = function(n, e) {
      var r = n[e];
      return r != null && n.hasOwnProperty(e) ? typeof r != "object" || (Array.isArray(r) ? r.length : Object.keys(r).length) > 0 : !1;
    }, f.Buffer = function() {
      try {
        var d = f.inquire("buffer").Buffer;
        return d.prototype.utf8Write ? d : (
          /* istanbul ignore next */
          null
        );
      } catch {
        return null;
      }
    }(), f._Buffer_from = null, f._Buffer_allocUnsafe = null, f.newBuffer = function(n) {
      return typeof n == "number" ? f.Buffer ? f._Buffer_allocUnsafe(n) : new f.Array(n) : f.Buffer ? f._Buffer_from(n) : typeof Uint8Array > "u" ? n : new Uint8Array(n);
    }, f.Array = typeof Uint8Array < "u" ? Uint8Array : Array, f.Long = /* istanbul ignore next */
    f.global.dcodeIO && /* istanbul ignore next */
    f.global.dcodeIO.Long || /* istanbul ignore next */
    f.global.Long || f.inquire("long"), f.key2Re = /^true|false|0|1$/, f.key32Re = /^-?(?:0|[1-9][0-9]*)$/, f.key64Re = /^(?:[\\\\x00-\\\\xff]{8}|-?(?:0|[1-9][0-9]*))$/, f.longToHash = function(n) {
      return n ? f.LongBits.from(n).toHash() : f.LongBits.zeroHash;
    }, f.longFromHash = function(n, e) {
      var r = f.LongBits.fromHash(n);
      return f.Long ? f.Long.fromBits(r.lo, r.hi, e) : r.toNumber(!!e);
    };
    function h(d, n, e) {
      for (var r = Object.keys(n), i = 0; i < r.length; ++i)
        (d[r[i]] === void 0 || !e) && (d[r[i]] = n[r[i]]);
      return d;
    }
    f.merge = h, f.lcFirst = function(n) {
      return n.charAt(0).toLowerCase() + n.substring(1);
    };
    function c(d) {
      function n(e, r) {
        if (!(this instanceof n))
          return new n(e, r);
        Object.defineProperty(this, "message", { get: function() {
          return e;
        } }), Error.captureStackTrace ? Error.captureStackTrace(this, n) : Object.defineProperty(this, "stack", { value: new Error().stack || "" }), r && h(this, r);
      }
      return n.prototype = Object.create(Error.prototype, {
        constructor: {
          value: n,
          writable: !0,
          enumerable: !1,
          configurable: !0
        },
        name: {
          get: function() {
            return d;
          },
          set: void 0,
          enumerable: !1,
          // configurable: false would accurately preserve the behavior of
          // the original, but I'm guessing that was not intentional.
          // For an actual error subclass, this property would
          // be configurable.
          configurable: !0
        },
        toString: {
          value: function() {
            return this.name + ": " + this.message;
          },
          writable: !0,
          enumerable: !1,
          configurable: !0
        }
      }), n;
    }
    f.newError = c, f.ProtocolError = c("ProtocolError"), f.oneOfGetter = function(n) {
      for (var e = {}, r = 0; r < n.length; ++r)
        e[n[r]] = 1;
      return function() {
        for (var i = Object.keys(this), t = i.length - 1; t > -1; --t)
          if (e[i[t]] === 1 && this[i[t]] !== void 0 && this[i[t]] !== null)
            return i[t];
      };
    }, f.oneOfSetter = function(n) {
      return function(e) {
        for (var r = 0; r < n.length; ++r)
          n[r] !== e && delete this[n[r]];
      };
    }, f.toJSONOptions = {
      longs: String,
      enums: String,
      bytes: String,
      json: !0
    }, f._configure = function() {
      var d = f.Buffer;
      if (!d) {
        f._Buffer_from = f._Buffer_allocUnsafe = null;
        return;
      }
      f._Buffer_from = d.from !== Uint8Array.from && d.from || /* istanbul ignore next */
      function(e, r) {
        return new d(e, r);
      }, f._Buffer_allocUnsafe = d.allocUnsafe || /* istanbul ignore next */
      function(e) {
        return new d(e);
      };
    };
  }(minimal)), minimal;
}
var writer, hasRequiredWriter;
function requireWriter() {
  if (hasRequiredWriter) return writer;
  hasRequiredWriter = 1, writer = i;
  var a = requireMinimal(), f, h = a.LongBits, c = a.base64, d = a.utf8;
  function n(E, v, m) {
    this.fn = E, this.len = v, this.next = void 0, this.val = m;
  }
  function e() {
  }
  function r(E) {
    this.head = E.head, this.tail = E.tail, this.len = E.len, this.next = E.states;
  }
  function i() {
    this.len = 0, this.head = new n(e, 0, 0), this.tail = this.head, this.states = null;
  }
  var t = function() {
    return a.Buffer ? function() {
      return (i.create = function() {
        return new f();
      })();
    } : function() {
      return new i();
    };
  };
  i.create = t(), i.alloc = function(v) {
    return new a.Array(v);
  }, a.Array !== Array && (i.alloc = a.pool(i.alloc, a.Array.prototype.subarray)), i.prototype._push = function(v, m, _) {
    return this.tail = this.tail.next = new n(v, m, _), this.len += m, this;
  };
  function l(E, v, m) {
    v[m] = E & 255;
  }
  function s(E, v, m) {
    for (; E > 127; )
      v[m++] = E & 127 | 128, E >>>= 7;
    v[m] = E;
  }
  function u(E, v) {
    this.len = E, this.next = void 0, this.val = v;
  }
  u.prototype = Object.create(n.prototype), u.prototype.fn = s, i.prototype.uint32 = function(v) {
    return this.len += (this.tail = this.tail.next = new u(
      (v = v >>> 0) < 128 ? 1 : v < 16384 ? 2 : v < 2097152 ? 3 : v < 268435456 ? 4 : 5,
      v
    )).len, this;
  }, i.prototype.int32 = function(v) {
    return v < 0 ? this._push(o, 10, h.fromNumber(v)) : this.uint32(v);
  }, i.prototype.sint32 = function(v) {
    return this.uint32((v << 1 ^ v >> 31) >>> 0);
  };
  function o(E, v, m) {
    for (; E.hi; )
      v[m++] = E.lo & 127 | 128, E.lo = (E.lo >>> 7 | E.hi << 25) >>> 0, E.hi >>>= 7;
    for (; E.lo > 127; )
      v[m++] = E.lo & 127 | 128, E.lo = E.lo >>> 7;
    v[m++] = E.lo;
  }
  i.prototype.uint64 = function(v) {
    var m = h.from(v);
    return this._push(o, m.length(), m);
  }, i.prototype.int64 = i.prototype.uint64, i.prototype.sint64 = function(v) {
    var m = h.from(v).zzEncode();
    return this._push(o, m.length(), m);
  }, i.prototype.bool = function(v) {
    return this._push(l, 1, v ? 1 : 0);
  };
  function p(E, v, m) {
    v[m] = E & 255, v[m + 1] = E >>> 8 & 255, v[m + 2] = E >>> 16 & 255, v[m + 3] = E >>> 24;
  }
  i.prototype.fixed32 = function(v) {
    return this._push(p, 4, v >>> 0);
  }, i.prototype.sfixed32 = i.prototype.fixed32, i.prototype.fixed64 = function(v) {
    var m = h.from(v);
    return this._push(p, 4, m.lo)._push(p, 4, m.hi);
  }, i.prototype.sfixed64 = i.prototype.fixed64, i.prototype.float = function(v) {
    return this._push(a.float.writeFloatLE, 4, v);
  }, i.prototype.double = function(v) {
    return this._push(a.float.writeDoubleLE, 8, v);
  };
  var y = a.Array.prototype.set ? function(v, m, _) {
    m.set(v, _);
  } : function(v, m, _) {
    for (var A = 0; A < v.length; ++A)
      m[_ + A] = v[A];
  };
  return i.prototype.bytes = function(v) {
    var m = v.length >>> 0;
    if (!m)
      return this._push(l, 1, 0);
    if (a.isString(v)) {
      var _ = i.alloc(m = c.length(v));
      c.decode(v, _, 0), v = _;
    }
    return this.uint32(m)._push(y, m, v);
  }, i.prototype.string = function(v) {
    var m = d.length(v);
    return m ? this.uint32(m)._push(d.write, m, v) : this._push(l, 1, 0);
  }, i.prototype.fork = function() {
    return this.states = new r(this), this.head = this.tail = new n(e, 0, 0), this.len = 0, this;
  }, i.prototype.reset = function() {
    return this.states ? (this.head = this.states.head, this.tail = this.states.tail, this.len = this.states.len, this.states = this.states.next) : (this.head = this.tail = new n(e, 0, 0), this.len = 0), this;
  }, i.prototype.ldelim = function() {
    var v = this.head, m = this.tail, _ = this.len;
    return this.reset().uint32(_), _ && (this.tail.next = v.next, this.tail = m, this.len += _), this;
  }, i.prototype.finish = function() {
    for (var v = this.head.next, m = this.constructor.alloc(this.len), _ = 0; v; )
      v.fn(v.val, m, _), _ += v.len, v = v.next;
    return m;
  }, i._configure = function(E) {
    f = E, i.create = t(), f._configure();
  }, writer;
}
var writer_buffer, hasRequiredWriter_buffer;
function requireWriter_buffer() {
  if (hasRequiredWriter_buffer) return writer_buffer;
  hasRequiredWriter_buffer = 1, writer_buffer = h;
  var a = requireWriter();
  (h.prototype = Object.create(a.prototype)).constructor = h;
  var f = requireMinimal();
  function h() {
    a.call(this);
  }
  h._configure = function() {
    h.alloc = f._Buffer_allocUnsafe, h.writeBytesBuffer = f.Buffer && f.Buffer.prototype instanceof Uint8Array && f.Buffer.prototype.set.name === "set" ? function(n, e, r) {
      e.set(n, r);
    } : function(n, e, r) {
      if (n.copy)
        n.copy(e, r, 0, n.length);
      else for (var i = 0; i < n.length; )
        e[r++] = n[i++];
    };
  }, h.prototype.bytes = function(n) {
    f.isString(n) && (n = f._Buffer_from(n, "base64"));
    var e = n.length >>> 0;
    return this.uint32(e), e && this._push(h.writeBytesBuffer, e, n), this;
  };
  function c(d, n, e) {
    d.length < 40 ? f.utf8.write(d, n, e) : n.utf8Write ? n.utf8Write(d, e) : n.write(d, e);
  }
  return h.prototype.string = function(n) {
    var e = f.Buffer.byteLength(n);
    return this.uint32(e), e && this._push(c, e, n), this;
  }, h._configure(), writer_buffer;
}
var reader, hasRequiredReader;
function requireReader() {
  if (hasRequiredReader) return reader;
  hasRequiredReader = 1, reader = n;
  var a = requireMinimal(), f, h = a.LongBits, c = a.utf8;
  function d(s, u) {
    return RangeError("index out of range: " + s.pos + " + " + (u || 1) + " > " + s.len);
  }
  function n(s) {
    this.buf = s, this.pos = 0, this.len = s.length;
  }
  var e = typeof Uint8Array < "u" ? function(u) {
    if (u instanceof Uint8Array || Array.isArray(u))
      return new n(u);
    throw Error("illegal buffer");
  } : function(u) {
    if (Array.isArray(u))
      return new n(u);
    throw Error("illegal buffer");
  }, r = function() {
    return a.Buffer ? function(o) {
      return (n.create = function(y) {
        return a.Buffer.isBuffer(y) ? new f(y) : e(y);
      })(o);
    } : e;
  };
  n.create = r(), n.prototype._slice = a.Array.prototype.subarray || /* istanbul ignore next */
  a.Array.prototype.slice, n.prototype.uint32 = /* @__PURE__ */ function() {
    var u = 4294967295;
    return function() {
      if (u = (this.buf[this.pos] & 127) >>> 0, this.buf[this.pos++] < 128 || (u = (u | (this.buf[this.pos] & 127) << 7) >>> 0, this.buf[this.pos++] < 128) || (u = (u | (this.buf[this.pos] & 127) << 14) >>> 0, this.buf[this.pos++] < 128) || (u = (u | (this.buf[this.pos] & 127) << 21) >>> 0, this.buf[this.pos++] < 128) || (u = (u | (this.buf[this.pos] & 15) << 28) >>> 0, this.buf[this.pos++] < 128)) return u;
      if ((this.pos += 5) > this.len)
        throw this.pos = this.len, d(this, 10);
      return u;
    };
  }(), n.prototype.int32 = function() {
    return this.uint32() | 0;
  }, n.prototype.sint32 = function() {
    var u = this.uint32();
    return u >>> 1 ^ -(u & 1) | 0;
  };
  function i() {
    var s = new h(0, 0), u = 0;
    if (this.len - this.pos > 4) {
      for (; u < 4; ++u)
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << u * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      if (s.lo = (s.lo | (this.buf[this.pos] & 127) << 28) >>> 0, s.hi = (s.hi | (this.buf[this.pos] & 127) >> 4) >>> 0, this.buf[this.pos++] < 128)
        return s;
      u = 0;
    } else {
      for (; u < 3; ++u) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << u * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
      return s.lo = (s.lo | (this.buf[this.pos++] & 127) << u * 7) >>> 0, s;
    }
    if (this.len - this.pos > 4) {
      for (; u < 5; ++u)
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << u * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
    } else
      for (; u < 5; ++u) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << u * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
    throw Error("invalid varint encoding");
  }
  n.prototype.bool = function() {
    return this.uint32() !== 0;
  };
  function t(s, u) {
    return (s[u - 4] | s[u - 3] << 8 | s[u - 2] << 16 | s[u - 1] << 24) >>> 0;
  }
  n.prototype.fixed32 = function() {
    if (this.pos + 4 > this.len)
      throw d(this, 4);
    return t(this.buf, this.pos += 4);
  }, n.prototype.sfixed32 = function() {
    if (this.pos + 4 > this.len)
      throw d(this, 4);
    return t(this.buf, this.pos += 4) | 0;
  };
  function l() {
    if (this.pos + 8 > this.len)
      throw d(this, 8);
    return new h(t(this.buf, this.pos += 4), t(this.buf, this.pos += 4));
  }
  return n.prototype.float = function() {
    if (this.pos + 4 > this.len)
      throw d(this, 4);
    var u = a.float.readFloatLE(this.buf, this.pos);
    return this.pos += 4, u;
  }, n.prototype.double = function() {
    if (this.pos + 8 > this.len)
      throw d(this, 4);
    var u = a.float.readDoubleLE(this.buf, this.pos);
    return this.pos += 8, u;
  }, n.prototype.bytes = function() {
    var u = this.uint32(), o = this.pos, p = this.pos + u;
    if (p > this.len)
      throw d(this, u);
    if (this.pos += u, Array.isArray(this.buf))
      return this.buf.slice(o, p);
    if (o === p) {
      var y = a.Buffer;
      return y ? y.alloc(0) : new this.buf.constructor(0);
    }
    return this._slice.call(this.buf, o, p);
  }, n.prototype.string = function() {
    var u = this.bytes();
    return c.read(u, 0, u.length);
  }, n.prototype.skip = function(u) {
    if (typeof u == "number") {
      if (this.pos + u > this.len)
        throw d(this, u);
      this.pos += u;
    } else
      do
        if (this.pos >= this.len)
          throw d(this);
      while (this.buf[this.pos++] & 128);
    return this;
  }, n.prototype.skipType = function(s) {
    switch (s) {
      case 0:
        this.skip();
        break;
      case 1:
        this.skip(8);
        break;
      case 2:
        this.skip(this.uint32());
        break;
      case 3:
        for (; (s = this.uint32() & 7) !== 4; )
          this.skipType(s);
        break;
      case 5:
        this.skip(4);
        break;
      /* istanbul ignore next */
      default:
        throw Error("invalid wire type " + s + " at offset " + this.pos);
    }
    return this;
  }, n._configure = function(s) {
    f = s, n.create = r(), f._configure();
    var u = a.Long ? "toLong" : (
      /* istanbul ignore next */
      "toNumber"
    );
    a.merge(n.prototype, {
      int64: function() {
        return i.call(this)[u](!1);
      },
      uint64: function() {
        return i.call(this)[u](!0);
      },
      sint64: function() {
        return i.call(this).zzDecode()[u](!1);
      },
      fixed64: function() {
        return l.call(this)[u](!0);
      },
      sfixed64: function() {
        return l.call(this)[u](!1);
      }
    });
  }, reader;
}
var reader_buffer, hasRequiredReader_buffer;
function requireReader_buffer() {
  if (hasRequiredReader_buffer) return reader_buffer;
  hasRequiredReader_buffer = 1, reader_buffer = h;
  var a = requireReader();
  (h.prototype = Object.create(a.prototype)).constructor = h;
  var f = requireMinimal();
  function h(c) {
    a.call(this, c);
  }
  return h._configure = function() {
    f.Buffer && (h.prototype._slice = f.Buffer.prototype.slice);
  }, h.prototype.string = function() {
    var d = this.uint32();
    return this.buf.utf8Slice ? this.buf.utf8Slice(this.pos, this.pos = Math.min(this.pos + d, this.len)) : this.buf.toString("utf-8", this.pos, this.pos = Math.min(this.pos + d, this.len));
  }, h._configure(), reader_buffer;
}
var rpc = {}, service$1, hasRequiredService$1;
function requireService$1() {
  if (hasRequiredService$1) return service$1;
  hasRequiredService$1 = 1, service$1 = f;
  var a = requireMinimal();
  (f.prototype = Object.create(a.EventEmitter.prototype)).constructor = f;
  function f(h, c, d) {
    if (typeof h != "function")
      throw TypeError("rpcImpl must be a function");
    a.EventEmitter.call(this), this.rpcImpl = h, this.requestDelimited = !!c, this.responseDelimited = !!d;
  }
  return f.prototype.rpcCall = function h(c, d, n, e, r) {
    if (!e)
      throw TypeError("request must be specified");
    var i = this;
    if (!r)
      return a.asPromise(h, i, c, d, n, e);
    if (!i.rpcImpl) {
      setTimeout(function() {
        r(Error("already ended"));
      }, 0);
      return;
    }
    try {
      return i.rpcImpl(
        c,
        d[i.requestDelimited ? "encodeDelimited" : "encode"](e).finish(),
        function(l, s) {
          if (l)
            return i.emit("error", l, c), r(l);
          if (s === null) {
            i.end(
              /* endedByRPC */
              !0
            );
            return;
          }
          if (!(s instanceof n))
            try {
              s = n[i.responseDelimited ? "decodeDelimited" : "decode"](s);
            } catch (u) {
              return i.emit("error", u, c), r(u);
            }
          return i.emit("data", s, c), r(null, s);
        }
      );
    } catch (t) {
      i.emit("error", t, c), setTimeout(function() {
        r(t);
      }, 0);
      return;
    }
  }, f.prototype.end = function(c) {
    return this.rpcImpl && (c || this.rpcImpl(null, null, null), this.rpcImpl = null, this.emit("end").off()), this;
  }, service$1;
}
var hasRequiredRpc;
function requireRpc() {
  return hasRequiredRpc || (hasRequiredRpc = 1, function(a) {
    var f = a;
    f.Service = requireService$1();
  }(rpc)), rpc;
}
var roots, hasRequiredRoots;
function requireRoots() {
  return hasRequiredRoots || (hasRequiredRoots = 1, roots = {}), roots;
}
var hasRequiredIndexMinimal;
function requireIndexMinimal() {
  return hasRequiredIndexMinimal || (hasRequiredIndexMinimal = 1, function(a) {
    var f = a;
    f.build = "minimal", f.Writer = requireWriter(), f.BufferWriter = requireWriter_buffer(), f.Reader = requireReader(), f.BufferReader = requireReader_buffer(), f.util = requireMinimal(), f.rpc = requireRpc(), f.roots = requireRoots(), f.configure = h;
    function h() {
      f.util._configure(), f.Writer._configure(f.BufferWriter), f.Reader._configure(f.BufferReader);
    }
    h();
  }(indexMinimal)), indexMinimal;
}
var types = {}, util = { exports: {} }, codegen_1, hasRequiredCodegen;
function requireCodegen() {
  if (hasRequiredCodegen) return codegen_1;
  hasRequiredCodegen = 1, codegen_1 = a;
  function a(f, h) {
    typeof f == "string" && (h = f, f = void 0);
    var c = [];
    function d(e) {
      if (typeof e != "string") {
        var r = n();
        if (a.verbose && console.log("codegen: " + r), r = "return " + r, e) {
          for (var i = Object.keys(e), t = new Array(i.length + 1), l = new Array(i.length), s = 0; s < i.length; )
            t[s] = i[s], l[s] = e[i[s++]];
          return t[s] = r, Function.apply(null, t).apply(null, l);
        }
        return Function(r)();
      }
      for (var u = new Array(arguments.length - 1), o = 0; o < u.length; )
        u[o] = arguments[++o];
      if (o = 0, e = e.replace(/%([%dfijs])/g, function(y, E) {
        var v = u[o++];
        switch (E) {
          case "d":
          case "f":
            return String(Number(v));
          case "i":
            return String(Math.floor(v));
          case "j":
            return JSON.stringify(v);
          case "s":
            return String(v);
        }
        return "%";
      }), o !== u.length)
        throw Error("parameter count mismatch");
      return c.push(e), d;
    }
    function n(e) {
      return "function " + (e || h || "") + "(" + (f && f.join(",") || "") + \`){
  \` + c.join(\`
  \`) + \`
}\`;
    }
    return d.toString = n, d;
  }
  return a.verbose = !1, codegen_1;
}
var fetch_1, hasRequiredFetch;
function requireFetch() {
  if (hasRequiredFetch) return fetch_1;
  hasRequiredFetch = 1, fetch_1 = c;
  var a = requireAspromise(), f = requireInquire(), h = f("fs");
  function c(d, n, e) {
    return typeof n == "function" ? (e = n, n = {}) : n || (n = {}), e ? !n.xhr && h && h.readFile ? h.readFile(d, function(i, t) {
      return i && typeof XMLHttpRequest < "u" ? c.xhr(d, n, e) : i ? e(i) : e(null, n.binary ? t : t.toString("utf8"));
    }) : c.xhr(d, n, e) : a(c, this, d, n);
  }
  return c.xhr = function(n, e, r) {
    var i = new XMLHttpRequest();
    i.onreadystatechange = function() {
      if (i.readyState === 4) {
        if (i.status !== 0 && i.status !== 200)
          return r(Error("status " + i.status));
        if (e.binary) {
          var l = i.response;
          if (!l) {
            l = [];
            for (var s = 0; s < i.responseText.length; ++s)
              l.push(i.responseText.charCodeAt(s) & 255);
          }
          return r(null, typeof Uint8Array < "u" ? new Uint8Array(l) : l);
        }
        return r(null, i.responseText);
      }
    }, e.binary && ("overrideMimeType" in i && i.overrideMimeType("text/plain; charset=x-user-defined"), i.responseType = "arraybuffer"), i.open("GET", n), i.send();
  }, fetch_1;
}
var path = {}, hasRequiredPath;
function requirePath() {
  return hasRequiredPath || (hasRequiredPath = 1, function(a) {
    var f = a, h = (
      /**
       * Tests if the specified path is absolute.
       * @param {string} path Path to test
       * @returns {boolean} \`true\` if path is absolute
       */
      f.isAbsolute = function(n) {
        return /^(?:\\/|\\w+:)/.test(n);
      }
    ), c = (
      /**
       * Normalizes the specified path.
       * @param {string} path Path to normalize
       * @returns {string} Normalized path
       */
      f.normalize = function(n) {
        n = n.replace(/\\\\/g, "/").replace(/\\/{2,}/g, "/");
        var e = n.split("/"), r = h(n), i = "";
        r && (i = e.shift() + "/");
        for (var t = 0; t < e.length; )
          e[t] === ".." ? t > 0 && e[t - 1] !== ".." ? e.splice(--t, 2) : r ? e.splice(t, 1) : ++t : e[t] === "." ? e.splice(t, 1) : ++t;
        return i + e.join("/");
      }
    );
    f.resolve = function(n, e, r) {
      return r || (e = c(e)), h(e) ? e : (r || (n = c(n)), (n = n.replace(/(?:\\/|^)[^/]+$/, "")).length ? c(n + "/" + e) : e);
    };
  }(path)), path;
}
var namespace, hasRequiredNamespace;
function requireNamespace() {
  if (hasRequiredNamespace) return namespace;
  hasRequiredNamespace = 1, namespace = i;
  var a = requireObject();
  ((i.prototype = Object.create(a.prototype)).constructor = i).className = "Namespace";
  var f = requireField(), h = requireUtil(), c = requireOneof(), d, n, e;
  i.fromJSON = function(s, u) {
    return new i(s, u.options).addJSON(u.nested);
  };
  function r(l, s) {
    if (l && l.length) {
      for (var u = {}, o = 0; o < l.length; ++o)
        u[l[o].name] = l[o].toJSON(s);
      return u;
    }
  }
  i.arrayToJSON = r, i.isReservedId = function(s, u) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (typeof s[o] != "string" && s[o][0] <= u && s[o][1] > u)
          return !0;
    }
    return !1;
  }, i.isReservedName = function(s, u) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (s[o] === u)
          return !0;
    }
    return !1;
  };
  function i(l, s) {
    a.call(this, l, s), this.nested = void 0, this._nestedArray = null, this._lookupCache = {}, this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
  }
  function t(l) {
    l._nestedArray = null, l._lookupCache = {};
    for (var s = l; s = s.parent; )
      s._lookupCache = {};
    return l;
  }
  return Object.defineProperty(i.prototype, "nestedArray", {
    get: function() {
      return this._nestedArray || (this._nestedArray = h.toArray(this.nested));
    }
  }), i.prototype.toJSON = function(s) {
    return h.toObject([
      "options",
      this.options,
      "nested",
      r(this.nestedArray, s)
    ]);
  }, i.prototype.addJSON = function(s) {
    var u = this;
    if (s)
      for (var o = Object.keys(s), p = 0, y; p < o.length; ++p)
        y = s[o[p]], u.add(
          // most to least likely
          (y.fields !== void 0 ? d.fromJSON : y.values !== void 0 ? e.fromJSON : y.methods !== void 0 ? n.fromJSON : y.id !== void 0 ? f.fromJSON : i.fromJSON)(o[p], y)
        );
    return this;
  }, i.prototype.get = function(s) {
    return this.nested && this.nested[s] || null;
  }, i.prototype.getEnum = function(s) {
    if (this.nested && this.nested[s] instanceof e)
      return this.nested[s].values;
    throw Error("no such enum: " + s);
  }, i.prototype.add = function(s) {
    if (!(s instanceof f && s.extend !== void 0 || s instanceof d || s instanceof c || s instanceof e || s instanceof n || s instanceof i))
      throw TypeError("object must be a valid nested object");
    if (!this.nested)
      this.nested = {};
    else {
      var u = this.get(s.name);
      if (u)
        if (u instanceof i && s instanceof i && !(u instanceof d || u instanceof n)) {
          for (var o = u.nestedArray, p = 0; p < o.length; ++p)
            s.add(o[p]);
          this.remove(u), this.nested || (this.nested = {}), s.setOptions(u.options, !0);
        } else
          throw Error("duplicate name '" + s.name + "' in " + this);
    }
    this.nested[s.name] = s, this instanceof d || this instanceof n || this instanceof e || this instanceof f || s._edition || (s._edition = s._defaultEdition), this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
    for (var y = this; y = y.parent; )
      y._needsRecursiveFeatureResolution = !0, y._needsRecursiveResolve = !0;
    return s.onAdd(this), t(this);
  }, i.prototype.remove = function(s) {
    if (!(s instanceof a))
      throw TypeError("object must be a ReflectionObject");
    if (s.parent !== this)
      throw Error(s + " is not a member of " + this);
    return delete this.nested[s.name], Object.keys(this.nested).length || (this.nested = void 0), s.onRemove(this), t(this);
  }, i.prototype.define = function(s, u) {
    if (h.isString(s))
      s = s.split(".");
    else if (!Array.isArray(s))
      throw TypeError("illegal path");
    if (s && s.length && s[0] === "")
      throw Error("path must be relative");
    for (var o = this; s.length > 0; ) {
      var p = s.shift();
      if (o.nested && o.nested[p]) {
        if (o = o.nested[p], !(o instanceof i))
          throw Error("path conflicts with non-namespace objects");
      } else
        o.add(o = new i(p));
    }
    return u && o.addJSON(u), o;
  }, i.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    this._resolveFeaturesRecursive(this._edition);
    var s = this.nestedArray, u = 0;
    for (this.resolve(); u < s.length; )
      s[u] instanceof i ? s[u++].resolveAll() : s[u++].resolve();
    return this._needsRecursiveResolve = !1, this;
  }, i.prototype._resolveFeaturesRecursive = function(s) {
    return this._needsRecursiveFeatureResolution ? (this._needsRecursiveFeatureResolution = !1, s = this._edition || s, a.prototype._resolveFeaturesRecursive.call(this, s), this.nestedArray.forEach((u) => {
      u._resolveFeaturesRecursive(s);
    }), this) : this;
  }, i.prototype.lookup = function(s, u, o) {
    if (typeof u == "boolean" ? (o = u, u = void 0) : u && !Array.isArray(u) && (u = [u]), h.isString(s) && s.length) {
      if (s === ".")
        return this.root;
      s = s.split(".");
    } else if (!s.length)
      return this;
    var p = s.join(".");
    if (s[0] === "")
      return this.root.lookup(s.slice(1), u);
    var y = this.root._fullyQualifiedObjects && this.root._fullyQualifiedObjects["." + p];
    if (y && (!u || u.indexOf(y.constructor) > -1) || (y = this._lookupImpl(s, p), y && (!u || u.indexOf(y.constructor) > -1)))
      return y;
    if (o)
      return null;
    for (var E = this; E.parent; ) {
      if (y = E.parent._lookupImpl(s, p), y && (!u || u.indexOf(y.constructor) > -1))
        return y;
      E = E.parent;
    }
    return null;
  }, i.prototype._lookupImpl = function(s, u) {
    if (Object.prototype.hasOwnProperty.call(this._lookupCache, u))
      return this._lookupCache[u];
    var o = this.get(s[0]), p = null;
    if (o)
      s.length === 1 ? p = o : o instanceof i && (s = s.slice(1), p = o._lookupImpl(s, s.join(".")));
    else
      for (var y = 0; y < this.nestedArray.length; ++y)
        this._nestedArray[y] instanceof i && (o = this._nestedArray[y]._lookupImpl(s, u)) && (p = o);
    return this._lookupCache[u] = p, p;
  }, i.prototype.lookupType = function(s) {
    var u = this.lookup(s, [d]);
    if (!u)
      throw Error("no such type: " + s);
    return u;
  }, i.prototype.lookupEnum = function(s) {
    var u = this.lookup(s, [e]);
    if (!u)
      throw Error("no such Enum '" + s + "' in " + this);
    return u;
  }, i.prototype.lookupTypeOrEnum = function(s) {
    var u = this.lookup(s, [d, e]);
    if (!u)
      throw Error("no such Type or Enum '" + s + "' in " + this);
    return u;
  }, i.prototype.lookupService = function(s) {
    var u = this.lookup(s, [n]);
    if (!u)
      throw Error("no such Service '" + s + "' in " + this);
    return u;
  }, i._configure = function(l, s, u) {
    d = l, n = s, e = u;
  }, namespace;
}
var mapfield, hasRequiredMapfield;
function requireMapfield() {
  if (hasRequiredMapfield) return mapfield;
  hasRequiredMapfield = 1, mapfield = c;
  var a = requireField();
  ((c.prototype = Object.create(a.prototype)).constructor = c).className = "MapField";
  var f = requireTypes(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (a.call(this, d, n, r, void 0, void 0, i, t), !h.isString(e))
      throw TypeError("keyType must be a string");
    this.keyType = e, this.resolvedKeyType = null, this.map = !0;
  }
  return c.fromJSON = function(n, e) {
    return new c(n, e.id, e.keyType, e.type, e.options, e.comment);
  }, c.prototype.toJSON = function(n) {
    var e = n ? !!n.keepComments : !1;
    return h.toObject([
      "keyType",
      this.keyType,
      "type",
      this.type,
      "id",
      this.id,
      "extend",
      this.extend,
      "options",
      this.options,
      "comment",
      e ? this.comment : void 0
    ]);
  }, c.prototype.resolve = function() {
    if (this.resolved)
      return this;
    if (f.mapKey[this.keyType] === void 0)
      throw Error("invalid key type: " + this.keyType);
    return a.prototype.resolve.call(this);
  }, c.d = function(n, e, r) {
    return typeof r == "function" ? r = h.decorateType(r).name : r && typeof r == "object" && (r = h.decorateEnum(r).name), function(t, l) {
      h.decorateType(t.constructor).add(new c(l, n, e, r));
    };
  }, mapfield;
}
var method, hasRequiredMethod;
function requireMethod() {
  if (hasRequiredMethod) return method;
  hasRequiredMethod = 1, method = h;
  var a = requireObject();
  ((h.prototype = Object.create(a.prototype)).constructor = h).className = "Method";
  var f = requireUtil();
  function h(c, d, n, e, r, i, t, l, s) {
    if (f.isObject(r) ? (t = r, r = i = void 0) : f.isObject(i) && (t = i, i = void 0), !(d === void 0 || f.isString(d)))
      throw TypeError("type must be a string");
    if (!f.isString(n))
      throw TypeError("requestType must be a string");
    if (!f.isString(e))
      throw TypeError("responseType must be a string");
    a.call(this, c, t), this.type = d || "rpc", this.requestType = n, this.requestStream = r ? !0 : void 0, this.responseType = e, this.responseStream = i ? !0 : void 0, this.resolvedRequestType = null, this.resolvedResponseType = null, this.comment = l, this.parsedOptions = s;
  }
  return h.fromJSON = function(d, n) {
    return new h(d, n.type, n.requestType, n.responseType, n.requestStream, n.responseStream, n.options, n.comment, n.parsedOptions);
  }, h.prototype.toJSON = function(d) {
    var n = d ? !!d.keepComments : !1;
    return f.toObject([
      "type",
      this.type !== "rpc" && /* istanbul ignore next */
      this.type || void 0,
      "requestType",
      this.requestType,
      "requestStream",
      this.requestStream,
      "responseType",
      this.responseType,
      "responseStream",
      this.responseStream,
      "options",
      this.options,
      "comment",
      n ? this.comment : void 0,
      "parsedOptions",
      this.parsedOptions
    ]);
  }, h.prototype.resolve = function() {
    return this.resolved ? this : (this.resolvedRequestType = this.parent.lookupType(this.requestType), this.resolvedResponseType = this.parent.lookupType(this.responseType), a.prototype.resolve.call(this));
  }, method;
}
var service, hasRequiredService;
function requireService() {
  if (hasRequiredService) return service;
  hasRequiredService = 1, service = d;
  var a = requireNamespace();
  ((d.prototype = Object.create(a.prototype)).constructor = d).className = "Service";
  var f = requireMethod(), h = requireUtil(), c = requireRpc();
  function d(e, r) {
    a.call(this, e, r), this.methods = {}, this._methodsArray = null;
  }
  d.fromJSON = function(r, i) {
    var t = new d(r, i.options);
    if (i.methods)
      for (var l = Object.keys(i.methods), s = 0; s < l.length; ++s)
        t.add(f.fromJSON(l[s], i.methods[l[s]]));
    return i.nested && t.addJSON(i.nested), i.edition && (t._edition = i.edition), t.comment = i.comment, t._defaultEdition = "proto3", t;
  }, d.prototype.toJSON = function(r) {
    var i = a.prototype.toJSON.call(this, r), t = r ? !!r.keepComments : !1;
    return h.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      i && i.options || void 0,
      "methods",
      a.arrayToJSON(this.methodsArray, r) || /* istanbul ignore next */
      {},
      "nested",
      i && i.nested || void 0,
      "comment",
      t ? this.comment : void 0
    ]);
  }, Object.defineProperty(d.prototype, "methodsArray", {
    get: function() {
      return this._methodsArray || (this._methodsArray = h.toArray(this.methods));
    }
  });
  function n(e) {
    return e._methodsArray = null, e;
  }
  return d.prototype.get = function(r) {
    return this.methods[r] || a.prototype.get.call(this, r);
  }, d.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    a.prototype.resolve.call(this);
    for (var r = this.methodsArray, i = 0; i < r.length; ++i)
      r[i].resolve();
    return this;
  }, d.prototype._resolveFeaturesRecursive = function(r) {
    return this._needsRecursiveFeatureResolution ? (r = this._edition || r, a.prototype._resolveFeaturesRecursive.call(this, r), this.methodsArray.forEach((i) => {
      i._resolveFeaturesRecursive(r);
    }), this) : this;
  }, d.prototype.add = function(r) {
    if (this.get(r.name))
      throw Error("duplicate name '" + r.name + "' in " + this);
    return r instanceof f ? (this.methods[r.name] = r, r.parent = this, n(this)) : a.prototype.add.call(this, r);
  }, d.prototype.remove = function(r) {
    if (r instanceof f) {
      if (this.methods[r.name] !== r)
        throw Error(r + " is not a member of " + this);
      return delete this.methods[r.name], r.parent = null, n(this);
    }
    return a.prototype.remove.call(this, r);
  }, d.prototype.create = function(r, i, t) {
    for (var l = new c.Service(r, i, t), s = 0, u; s < /* initializes */
    this.methodsArray.length; ++s) {
      var o = h.lcFirst((u = this._methodsArray[s]).resolve().name).replace(/[^$\\w_]/g, "");
      l[o] = h.codegen(["r", "c"], h.isReserved(o) ? o + "_" : o)("return this.rpcCall(m,q,s,r,c)")({
        m: u,
        q: u.resolvedRequestType.ctor,
        s: u.resolvedResponseType.ctor
      });
    }
    return l;
  }, service;
}
var message, hasRequiredMessage;
function requireMessage() {
  if (hasRequiredMessage) return message;
  hasRequiredMessage = 1, message = f;
  var a = requireMinimal();
  function f(h) {
    if (h)
      for (var c = Object.keys(h), d = 0; d < c.length; ++d) {
        var n = c[d];
        n !== "__proto__" && (this[n] = h[n]);
      }
  }
  return f.create = function(c) {
    return this.$type.create(c);
  }, f.encode = function(c, d) {
    return this.$type.encode(c, d);
  }, f.encodeDelimited = function(c, d) {
    return this.$type.encodeDelimited(c, d);
  }, f.decode = function(c) {
    return this.$type.decode(c);
  }, f.decodeDelimited = function(c) {
    return this.$type.decodeDelimited(c);
  }, f.verify = function(c) {
    return this.$type.verify(c);
  }, f.fromObject = function(c) {
    return this.$type.fromObject(c);
  }, f.toObject = function(c, d) {
    return this.$type.toObject(c, d);
  }, f.prototype.toJSON = function() {
    return this.$type.toObject(this, a.toJSONOptions);
  }, message;
}
var decoder_1, hasRequiredDecoder;
function requireDecoder() {
  if (hasRequiredDecoder) return decoder_1;
  hasRequiredDecoder = 1, decoder_1 = d;
  var a = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n) {
    return "missing required '" + n.name + "'";
  }
  function d(n) {
    for (var e = h.codegen(["r", "l", "e"], n.name + "$decode")("if(!(r instanceof Reader))")("r=Reader.create(r)")("var c=l===undefined?r.len:r.pos+l,m=new this.ctor" + (n.fieldsArray.filter(function(u) {
      return u.map;
    }).length ? ",k,value" : ""))("while(r.pos<c){")("var t=r.uint32()")("if(t===e)")("break")("switch(t>>>3){"), r = 0; r < /* initializes */
    n.fieldsArray.length; ++r) {
      var i = n._fieldsArray[r].resolve(), t = i.resolvedType instanceof a ? "int32" : i.type, l = "m" + h.safeProp(i.name);
      e("case %i: {", i.id), i.map ? (e("if(%s===util.emptyObject)", l)("%s={}", l)("var c2 = r.uint32()+r.pos"), f.defaults[i.keyType] !== void 0 ? e("k=%j", f.defaults[i.keyType]) : e("k=null"), f.defaults[t] !== void 0 ? e("value=%j", f.defaults[t]) : e("value=null"), e("while(r.pos<c2){")("var tag2=r.uint32()")("switch(tag2>>>3){")("case 1: k=r.%s(); break", i.keyType)("case 2:"), f.basic[t] === void 0 ? e("value=types[%i].decode(r,r.uint32())", r) : e("value=r.%s()", t), e("break")("default:")("r.skipType(tag2&7)")("break")("}")("}"), f.long[i.keyType] !== void 0 ? e('%s[typeof k==="object"?util.longToHash(k):k]=value', l) : e("%s[k]=value", l)) : i.repeated ? (e("if(!(%s&&%s.length))", l, l)("%s=[]", l), f.packed[t] !== void 0 && e("if((t&7)===2){")("var c2=r.uint32()+r.pos")("while(r.pos<c2)")("%s.push(r.%s())", l, t)("}else"), f.basic[t] === void 0 ? e(i.delimited ? "%s.push(types[%i].decode(r,undefined,((t&~7)|4)))" : "%s.push(types[%i].decode(r,r.uint32()))", l, r) : e("%s.push(r.%s())", l, t)) : f.basic[t] === void 0 ? e(i.delimited ? "%s=types[%i].decode(r,undefined,((t&~7)|4))" : "%s=types[%i].decode(r,r.uint32())", l, r) : e("%s=r.%s()", l, t), e("break")("}");
    }
    for (e("default:")("r.skipType(t&7)")("break")("}")("}"), r = 0; r < n._fieldsArray.length; ++r) {
      var s = n._fieldsArray[r];
      s.required && e("if(!m.hasOwnProperty(%j))", s.name)("throw util.ProtocolError(%j,{instance:m})", c(s));
    }
    return e("return m");
  }
  return decoder_1;
}
var verifier_1, hasRequiredVerifier;
function requireVerifier() {
  if (hasRequiredVerifier) return verifier_1;
  hasRequiredVerifier = 1, verifier_1 = n;
  var a = require_enum(), f = requireUtil();
  function h(e, r) {
    return e.name + ": " + r + (e.repeated && r !== "array" ? "[]" : e.map && r !== "object" ? "{k:" + e.keyType + "}" : "") + " expected";
  }
  function c(e, r, i, t) {
    if (r.resolvedType)
      if (r.resolvedType instanceof a) {
        e("switch(%s){", t)("default:")("return%j", h(r, "enum value"));
        for (var l = Object.keys(r.resolvedType.values), s = 0; s < l.length; ++s) e("case %i:", r.resolvedType.values[l[s]]);
        e("break")("}");
      } else
        e("{")("var e=types[%i].verify(%s);", i, t)("if(e)")("return%j+e", r.name + ".")("}");
    else
      switch (r.type) {
        case "int32":
        case "uint32":
        case "sint32":
        case "fixed32":
        case "sfixed32":
          e("if(!util.isInteger(%s))", t)("return%j", h(r, "integer"));
          break;
        case "int64":
        case "uint64":
        case "sint64":
        case "fixed64":
        case "sfixed64":
          e("if(!util.isInteger(%s)&&!(%s&&util.isInteger(%s.low)&&util.isInteger(%s.high)))", t, t, t, t)("return%j", h(r, "integer|Long"));
          break;
        case "float":
        case "double":
          e('if(typeof %s!=="number")', t)("return%j", h(r, "number"));
          break;
        case "bool":
          e('if(typeof %s!=="boolean")', t)("return%j", h(r, "boolean"));
          break;
        case "string":
          e("if(!util.isString(%s))", t)("return%j", h(r, "string"));
          break;
        case "bytes":
          e('if(!(%s&&typeof %s.length==="number"||util.isString(%s)))', t, t, t)("return%j", h(r, "buffer"));
          break;
      }
    return e;
  }
  function d(e, r, i) {
    switch (r.keyType) {
      case "int32":
      case "uint32":
      case "sint32":
      case "fixed32":
      case "sfixed32":
        e("if(!util.key32Re.test(%s))", i)("return%j", h(r, "integer key"));
        break;
      case "int64":
      case "uint64":
      case "sint64":
      case "fixed64":
      case "sfixed64":
        e("if(!util.key64Re.test(%s))", i)("return%j", h(r, "integer|Long key"));
        break;
      case "bool":
        e("if(!util.key2Re.test(%s))", i)("return%j", h(r, "boolean key"));
        break;
    }
    return e;
  }
  function n(e) {
    var r = f.codegen(["m"], e.name + "$verify")('if(typeof m!=="object"||m===null)')("return%j", "object expected"), i = e.oneofsArray, t = {};
    i.length && r("var p={}");
    for (var l = 0; l < /* initializes */
    e.fieldsArray.length; ++l) {
      var s = e._fieldsArray[l].resolve(), u = "m" + f.safeProp(s.name);
      if (s.optional && r("if(%s!=null&&m.hasOwnProperty(%j)){", u, s.name), s.map)
        r("if(!util.isObject(%s))", u)("return%j", h(s, "object"))("var k=Object.keys(%s)", u)("for(var i=0;i<k.length;++i){"), d(r, s, "k[i]"), c(r, s, l, u + "[k[i]]")("}");
      else if (s.repeated)
        r("if(!Array.isArray(%s))", u)("return%j", h(s, "array"))("for(var i=0;i<%s.length;++i){", u), c(r, s, l, u + "[i]")("}");
      else {
        if (s.partOf) {
          var o = f.safeProp(s.partOf.name);
          t[s.partOf.name] === 1 && r("if(p%s===1)", o)("return%j", s.partOf.name + ": multiple values"), t[s.partOf.name] = 1, r("p%s=1", o);
        }
        c(r, s, l, u);
      }
      s.optional && r("}");
    }
    return r("return null");
  }
  return verifier_1;
}
var converter = {}, hasRequiredConverter;
function requireConverter() {
  return hasRequiredConverter || (hasRequiredConverter = 1, function(a) {
    var f = a, h = require_enum(), c = requireUtil();
    function d(e, r, i, t) {
      var l = !1;
      if (r.resolvedType)
        if (r.resolvedType instanceof h) {
          e("switch(d%s){", t);
          for (var s = r.resolvedType.values, u = Object.keys(s), o = 0; o < u.length; ++o)
            s[u[o]] === r.typeDefault && !l && (e("default:")('if(typeof(d%s)==="number"){m%s=d%s;break}', t, t, t), r.repeated || e("break"), l = !0), e("case%j:", u[o])("case %i:", s[u[o]])("m%s=%j", t, s[u[o]])("break");
          e("}");
        } else e('if(typeof d%s!=="object")', t)("throw TypeError(%j)", r.fullName + ": object expected")("m%s=types[%i].fromObject(d%s)", t, i, t);
      else {
        var p = !1;
        switch (r.type) {
          case "double":
          case "float":
            e("m%s=Number(d%s)", t, t);
            break;
          case "uint32":
          case "fixed32":
            e("m%s=d%s>>>0", t, t);
            break;
          case "int32":
          case "sint32":
          case "sfixed32":
            e("m%s=d%s|0", t, t);
            break;
          case "uint64":
            p = !0;
          // eslint-disable-next-line no-fallthrough
          case "int64":
          case "sint64":
          case "fixed64":
          case "sfixed64":
            e("if(util.Long)")("(m%s=util.Long.fromValue(d%s)).unsigned=%j", t, t, p)('else if(typeof d%s==="string")', t)("m%s=parseInt(d%s,10)", t, t)('else if(typeof d%s==="number")', t)("m%s=d%s", t, t)('else if(typeof d%s==="object")', t)("m%s=new util.LongBits(d%s.low>>>0,d%s.high>>>0).toNumber(%s)", t, t, t, p ? "true" : "");
            break;
          case "bytes":
            e('if(typeof d%s==="string")', t)("util.base64.decode(d%s,m%s=util.newBuffer(util.base64.length(d%s)),0)", t, t, t)("else if(d%s.length >= 0)", t)("m%s=d%s", t, t);
            break;
          case "string":
            e("m%s=String(d%s)", t, t);
            break;
          case "bool":
            e("m%s=Boolean(d%s)", t, t);
            break;
        }
      }
      return e;
    }
    f.fromObject = function(r) {
      var i = r.fieldsArray, t = c.codegen(["d"], r.name + "$fromObject")("if(d instanceof this.ctor)")("return d");
      if (!i.length) return t("return new this.ctor");
      t("var m=new this.ctor");
      for (var l = 0; l < i.length; ++l) {
        var s = i[l].resolve(), u = c.safeProp(s.name);
        s.map ? (t("if(d%s){", u)('if(typeof d%s!=="object")', u)("throw TypeError(%j)", s.fullName + ": object expected")("m%s={}", u)("for(var ks=Object.keys(d%s),i=0;i<ks.length;++i){", u), d(
          t,
          s,
          /* not sorted */
          l,
          u + "[ks[i]]"
        )("}")("}")) : s.repeated ? (t("if(d%s){", u)("if(!Array.isArray(d%s))", u)("throw TypeError(%j)", s.fullName + ": array expected")("m%s=[]", u)("for(var i=0;i<d%s.length;++i){", u), d(
          t,
          s,
          /* not sorted */
          l,
          u + "[i]"
        )("}")("}")) : (s.resolvedType instanceof h || t("if(d%s!=null){", u), d(
          t,
          s,
          /* not sorted */
          l,
          u
        ), s.resolvedType instanceof h || t("}"));
      }
      return t("return m");
    };
    function n(e, r, i, t) {
      if (r.resolvedType)
        r.resolvedType instanceof h ? e("d%s=o.enums===String?(types[%i].values[m%s]===undefined?m%s:types[%i].values[m%s]):m%s", t, i, t, t, i, t, t) : e("d%s=types[%i].toObject(m%s,o)", t, i, t);
      else {
        var l = !1;
        switch (r.type) {
          case "double":
          case "float":
            e("d%s=o.json&&!isFinite(m%s)?String(m%s):m%s", t, t, t, t);
            break;
          case "uint64":
            l = !0;
          // eslint-disable-next-line no-fallthrough
          case "int64":
          case "sint64":
          case "fixed64":
          case "sfixed64":
            e('if(typeof m%s==="number")', t)("d%s=o.longs===String?String(m%s):m%s", t, t, t)("else")("d%s=o.longs===String?util.Long.prototype.toString.call(m%s):o.longs===Number?new util.LongBits(m%s.low>>>0,m%s.high>>>0).toNumber(%s):m%s", t, t, t, t, l ? "true" : "", t);
            break;
          case "bytes":
            e("d%s=o.bytes===String?util.base64.encode(m%s,0,m%s.length):o.bytes===Array?Array.prototype.slice.call(m%s):m%s", t, t, t, t, t);
            break;
          default:
            e("d%s=m%s", t, t);
            break;
        }
      }
      return e;
    }
    f.toObject = function(r) {
      var i = r.fieldsArray.slice().sort(c.compareFieldsById);
      if (!i.length)
        return c.codegen()("return {}");
      for (var t = c.codegen(["m", "o"], r.name + "$toObject")("if(!o)")("o={}")("var d={}"), l = [], s = [], u = [], o = 0; o < i.length; ++o)
        i[o].partOf || (i[o].resolve().repeated ? l : i[o].map ? s : u).push(i[o]);
      if (l.length) {
        for (t("if(o.arrays||o.defaults){"), o = 0; o < l.length; ++o) t("d%s=[]", c.safeProp(l[o].name));
        t("}");
      }
      if (s.length) {
        for (t("if(o.objects||o.defaults){"), o = 0; o < s.length; ++o) t("d%s={}", c.safeProp(s[o].name));
        t("}");
      }
      if (u.length) {
        for (t("if(o.defaults){"), o = 0; o < u.length; ++o) {
          var p = u[o], y = c.safeProp(p.name);
          if (p.resolvedType instanceof h) t("d%s=o.enums===String?%j:%j", y, p.resolvedType.valuesById[p.typeDefault], p.typeDefault);
          else if (p.long) t("if(util.Long){")("var n=new util.Long(%i,%i,%j)", p.typeDefault.low, p.typeDefault.high, p.typeDefault.unsigned)("d%s=o.longs===String?n.toString():o.longs===Number?n.toNumber():n", y)("}else")("d%s=o.longs===String?%j:%i", y, p.typeDefault.toString(), p.typeDefault.toNumber());
          else if (p.bytes) {
            var E = "[" + Array.prototype.slice.call(p.typeDefault).join(",") + "]";
            t("if(o.bytes===String)d%s=%j", y, String.fromCharCode.apply(String, p.typeDefault))("else{")("d%s=%s", y, E)("if(o.bytes!==Array)d%s=util.newBuffer(d%s)", y, y)("}");
          } else t("d%s=%j", y, p.typeDefault);
        }
        t("}");
      }
      var v = !1;
      for (o = 0; o < i.length; ++o) {
        var p = i[o], m = r._fieldsArray.indexOf(p), y = c.safeProp(p.name);
        p.map ? (v || (v = !0, t("var ks2")), t("if(m%s&&(ks2=Object.keys(m%s)).length){", y, y)("d%s={}", y)("for(var j=0;j<ks2.length;++j){"), n(
          t,
          p,
          /* sorted */
          m,
          y + "[ks2[j]]"
        )("}")) : p.repeated ? (t("if(m%s&&m%s.length){", y, y)("d%s=[]", y)("for(var j=0;j<m%s.length;++j){", y), n(
          t,
          p,
          /* sorted */
          m,
          y + "[j]"
        )("}")) : (t("if(m%s!=null&&m.hasOwnProperty(%j)){", y, p.name), n(
          t,
          p,
          /* sorted */
          m,
          y
        ), p.partOf && t("if(o.oneofs)")("d%s=%j", c.safeProp(p.partOf.name), p.name)), t("}");
      }
      return t("return d");
    };
  }(converter)), converter;
}
var wrappers = {}, hasRequiredWrappers;
function requireWrappers() {
  return hasRequiredWrappers || (hasRequiredWrappers = 1, function(a) {
    var f = a, h = requireMessage();
    f[".google.protobuf.Any"] = {
      fromObject: function(c) {
        if (c && c["@type"]) {
          var d = c["@type"].substring(c["@type"].lastIndexOf("/") + 1), n = this.lookup(d);
          if (n) {
            var e = c["@type"].charAt(0) === "." ? c["@type"].slice(1) : c["@type"];
            return e.indexOf("/") === -1 && (e = "/" + e), this.create({
              type_url: e,
              value: n.encode(n.fromObject(c)).finish()
            });
          }
        }
        return this.fromObject(c);
      },
      toObject: function(c, d) {
        var n = "type.googleapis.com/", e = "", r = "";
        if (d && d.json && c.type_url && c.value) {
          r = c.type_url.substring(c.type_url.lastIndexOf("/") + 1), e = c.type_url.substring(0, c.type_url.lastIndexOf("/") + 1);
          var i = this.lookup(r);
          i && (c = i.decode(c.value));
        }
        if (!(c instanceof this.ctor) && c instanceof h) {
          var t = c.$type.toObject(c, d), l = c.$type.fullName[0] === "." ? c.$type.fullName.slice(1) : c.$type.fullName;
          return e === "" && (e = n), r = e + l, t["@type"] = r, t;
        }
        return this.toObject(c, d);
      }
    };
  }(wrappers)), wrappers;
}
var type, hasRequiredType;
function requireType() {
  if (hasRequiredType) return type;
  hasRequiredType = 1, type = y;
  var a = requireNamespace();
  ((y.prototype = Object.create(a.prototype)).constructor = y).className = "Type";
  var f = require_enum(), h = requireOneof(), c = requireField(), d = requireMapfield(), n = requireService(), e = requireMessage(), r = requireReader(), i = requireWriter(), t = requireUtil(), l = requireEncoder(), s = requireDecoder(), u = requireVerifier(), o = requireConverter(), p = requireWrappers();
  function y(v, m) {
    v = v.replace(/\\W/g, ""), a.call(this, v, m), this.fields = {}, this.oneofs = void 0, this.extensions = void 0, this.reserved = void 0, this.group = void 0, this._fieldsById = null, this._fieldsArray = null, this._oneofsArray = null, this._ctor = null;
  }
  Object.defineProperties(y.prototype, {
    /**
     * Message fields by id.
     * @name Type#fieldsById
     * @type {Object.<number,Field>}
     * @readonly
     */
    fieldsById: {
      get: function() {
        if (this._fieldsById)
          return this._fieldsById;
        this._fieldsById = {};
        for (var v = Object.keys(this.fields), m = 0; m < v.length; ++m) {
          var _ = this.fields[v[m]], A = _.id;
          if (this._fieldsById[A])
            throw Error("duplicate id " + A + " in " + this);
          this._fieldsById[A] = _;
        }
        return this._fieldsById;
      }
    },
    /**
     * Fields of this message as an array for iteration.
     * @name Type#fieldsArray
     * @type {Field[]}
     * @readonly
     */
    fieldsArray: {
      get: function() {
        return this._fieldsArray || (this._fieldsArray = t.toArray(this.fields));
      }
    },
    /**
     * Oneofs of this message as an array for iteration.
     * @name Type#oneofsArray
     * @type {OneOf[]}
     * @readonly
     */
    oneofsArray: {
      get: function() {
        return this._oneofsArray || (this._oneofsArray = t.toArray(this.oneofs));
      }
    },
    /**
     * The registered constructor, if any registered, otherwise a generic constructor.
     * Assigning a function replaces the internal constructor. If the function does not extend {@link Message} yet, its prototype will be setup accordingly and static methods will be populated. If it already extends {@link Message}, it will just replace the internal constructor.
     * @name Type#ctor
     * @type {Constructor<{}>}
     */
    ctor: {
      get: function() {
        return this._ctor || (this.ctor = y.generateConstructor(this)());
      },
      set: function(v) {
        var m = v.prototype;
        m instanceof e || ((v.prototype = new e()).constructor = v, t.merge(v.prototype, m)), v.$type = v.prototype.$type = this, t.merge(v, e, !0), this._ctor = v;
        for (var _ = 0; _ < /* initializes */
        this.fieldsArray.length; ++_)
          this._fieldsArray[_].resolve();
        var A = {};
        for (_ = 0; _ < /* initializes */
        this.oneofsArray.length; ++_)
          A[this._oneofsArray[_].resolve().name] = {
            get: t.oneOfGetter(this._oneofsArray[_].oneof),
            set: t.oneOfSetter(this._oneofsArray[_].oneof)
          };
        _ && Object.defineProperties(v.prototype, A);
      }
    }
  }), y.generateConstructor = function(m) {
    for (var _ = t.codegen(["p"], m.name), A = 0, I; A < m.fieldsArray.length; ++A)
      (I = m._fieldsArray[A]).map ? _("this%s={}", t.safeProp(I.name)) : I.repeated && _("this%s=[]", t.safeProp(I.name));
    return _("if(p)for(var ks=Object.keys(p),i=0;i<ks.length;++i)if(p[ks[i]]!=null)")("this[ks[i]]=p[ks[i]]");
  };
  function E(v) {
    return v._fieldsById = v._fieldsArray = v._oneofsArray = null, delete v.encode, delete v.decode, delete v.verify, v;
  }
  return y.fromJSON = function(m, _) {
    var A = new y(m, _.options);
    A.extensions = _.extensions, A.reserved = _.reserved;
    for (var I = Object.keys(_.fields), C = 0; C < I.length; ++C)
      A.add(
        (typeof _.fields[I[C]].keyType < "u" ? d.fromJSON : c.fromJSON)(I[C], _.fields[I[C]])
      );
    if (_.oneofs)
      for (I = Object.keys(_.oneofs), C = 0; C < I.length; ++C)
        A.add(h.fromJSON(I[C], _.oneofs[I[C]]));
    if (_.nested)
      for (I = Object.keys(_.nested), C = 0; C < I.length; ++C) {
        var j = _.nested[I[C]];
        A.add(
          // most to least likely
          (j.id !== void 0 ? c.fromJSON : j.fields !== void 0 ? y.fromJSON : j.values !== void 0 ? f.fromJSON : j.methods !== void 0 ? n.fromJSON : a.fromJSON)(I[C], j)
        );
      }
    return _.extensions && _.extensions.length && (A.extensions = _.extensions), _.reserved && _.reserved.length && (A.reserved = _.reserved), _.group && (A.group = !0), _.comment && (A.comment = _.comment), _.edition && (A._edition = _.edition), A._defaultEdition = "proto3", A;
  }, y.prototype.toJSON = function(m) {
    var _ = a.prototype.toJSON.call(this, m), A = m ? !!m.keepComments : !1;
    return t.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      _ && _.options || void 0,
      "oneofs",
      a.arrayToJSON(this.oneofsArray, m),
      "fields",
      a.arrayToJSON(this.fieldsArray.filter(function(I) {
        return !I.declaringField;
      }), m) || {},
      "extensions",
      this.extensions && this.extensions.length ? this.extensions : void 0,
      "reserved",
      this.reserved && this.reserved.length ? this.reserved : void 0,
      "group",
      this.group || void 0,
      "nested",
      _ && _.nested || void 0,
      "comment",
      A ? this.comment : void 0
    ]);
  }, y.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    a.prototype.resolveAll.call(this);
    var m = this.oneofsArray;
    for (A = 0; A < m.length; )
      m[A++].resolve();
    for (var _ = this.fieldsArray, A = 0; A < _.length; )
      _[A++].resolve();
    return this;
  }, y.prototype._resolveFeaturesRecursive = function(m) {
    return this._needsRecursiveFeatureResolution ? (m = this._edition || m, a.prototype._resolveFeaturesRecursive.call(this, m), this.oneofsArray.forEach((_) => {
      _._resolveFeatures(m);
    }), this.fieldsArray.forEach((_) => {
      _._resolveFeatures(m);
    }), this) : this;
  }, y.prototype.get = function(m) {
    return this.fields[m] || this.oneofs && this.oneofs[m] || this.nested && this.nested[m] || null;
  }, y.prototype.add = function(m) {
    if (this.get(m.name))
      throw Error("duplicate name '" + m.name + "' in " + this);
    if (m instanceof c && m.extend === void 0) {
      if (this._fieldsById ? (
        /* istanbul ignore next */
        this._fieldsById[m.id]
      ) : this.fieldsById[m.id])
        throw Error("duplicate id " + m.id + " in " + this);
      if (this.isReservedId(m.id))
        throw Error("id " + m.id + " is reserved in " + this);
      if (this.isReservedName(m.name))
        throw Error("name '" + m.name + "' is reserved in " + this);
      return m.parent && m.parent.remove(m), this.fields[m.name] = m, m.message = this, m.onAdd(this), E(this);
    }
    return m instanceof h ? (this.oneofs || (this.oneofs = {}), this.oneofs[m.name] = m, m.onAdd(this), E(this)) : a.prototype.add.call(this, m);
  }, y.prototype.remove = function(m) {
    if (m instanceof c && m.extend === void 0) {
      if (!this.fields || this.fields[m.name] !== m)
        throw Error(m + " is not a member of " + this);
      return delete this.fields[m.name], m.parent = null, m.onRemove(this), E(this);
    }
    if (m instanceof h) {
      if (!this.oneofs || this.oneofs[m.name] !== m)
        throw Error(m + " is not a member of " + this);
      return delete this.oneofs[m.name], m.parent = null, m.onRemove(this), E(this);
    }
    return a.prototype.remove.call(this, m);
  }, y.prototype.isReservedId = function(m) {
    return a.isReservedId(this.reserved, m);
  }, y.prototype.isReservedName = function(m) {
    return a.isReservedName(this.reserved, m);
  }, y.prototype.create = function(m) {
    return new this.ctor(m);
  }, y.prototype.setup = function() {
    for (var m = this.fullName, _ = [], A = 0; A < /* initializes */
    this.fieldsArray.length; ++A)
      _.push(this._fieldsArray[A].resolve().resolvedType);
    this.encode = l(this)({
      Writer: i,
      types: _,
      util: t
    }), this.decode = s(this)({
      Reader: r,
      types: _,
      util: t
    }), this.verify = u(this)({
      types: _,
      util: t
    }), this.fromObject = o.fromObject(this)({
      types: _,
      util: t
    }), this.toObject = o.toObject(this)({
      types: _,
      util: t
    });
    var I = p[m];
    if (I) {
      var C = Object.create(this);
      C.fromObject = this.fromObject, this.fromObject = I.fromObject.bind(C), C.toObject = this.toObject, this.toObject = I.toObject.bind(C);
    }
    return this;
  }, y.prototype.encode = function(m, _) {
    return this.setup().encode(m, _);
  }, y.prototype.encodeDelimited = function(m, _) {
    return this.encode(m, _ && _.len ? _.fork() : _).ldelim();
  }, y.prototype.decode = function(m, _) {
    return this.setup().decode(m, _);
  }, y.prototype.decodeDelimited = function(m) {
    return m instanceof r || (m = r.create(m)), this.decode(m, m.uint32());
  }, y.prototype.verify = function(m) {
    return this.setup().verify(m);
  }, y.prototype.fromObject = function(m) {
    return this.setup().fromObject(m);
  }, y.prototype.toObject = function(m, _) {
    return this.setup().toObject(m, _);
  }, y.d = function(m) {
    return function(A) {
      t.decorateType(A, m);
    };
  }, type;
}
var root$1, hasRequiredRoot;
function requireRoot() {
  if (hasRequiredRoot) return root$1;
  hasRequiredRoot = 1, root$1 = i;
  var a = requireNamespace();
  ((i.prototype = Object.create(a.prototype)).constructor = i).className = "Root";
  var f = requireField(), h = require_enum(), c = requireOneof(), d = requireUtil(), n, e, r;
  function i(u) {
    a.call(this, "", u), this.deferred = [], this.files = [], this._edition = "proto2", this._fullyQualifiedObjects = {};
  }
  i.fromJSON = function(o, p) {
    return p || (p = new i()), o.options && p.setOptions(o.options), p.addJSON(o.nested).resolveAll();
  }, i.prototype.resolvePath = d.path.resolve, i.prototype.fetch = d.fetch;
  function t() {
  }
  i.prototype.load = function u(o, p, y) {
    typeof p == "function" && (y = p, p = void 0);
    var E = this;
    if (!y)
      return d.asPromise(u, E, o, p);
    var v = y === t;
    function m(B, L) {
      if (y) {
        if (v)
          throw B;
        L && L.resolveAll();
        var S = y;
        y = null, S(B, L);
      }
    }
    function _(B) {
      var L = B.lastIndexOf("google/protobuf/");
      if (L > -1) {
        var S = B.substring(L);
        if (S in r) return S;
      }
      return null;
    }
    function A(B, L) {
      try {
        if (d.isString(L) && L.charAt(0) === "{" && (L = JSON.parse(L)), !d.isString(L))
          E.setOptions(L.options).addJSON(L.nested);
        else {
          e.filename = B;
          var S = e(L, E, p), J, M = 0;
          if (S.imports)
            for (; M < S.imports.length; ++M)
              (J = _(S.imports[M]) || E.resolvePath(B, S.imports[M])) && I(J);
          if (S.weakImports)
            for (M = 0; M < S.weakImports.length; ++M)
              (J = _(S.weakImports[M]) || E.resolvePath(B, S.weakImports[M])) && I(J, !0);
        }
      } catch (T) {
        m(T);
      }
      !v && !C && m(null, E);
    }
    function I(B, L) {
      if (B = _(B) || B, !(E.files.indexOf(B) > -1)) {
        if (E.files.push(B), B in r) {
          v ? A(B, r[B]) : (++C, setTimeout(function() {
            --C, A(B, r[B]);
          }));
          return;
        }
        if (v) {
          var S;
          try {
            S = d.fs.readFileSync(B).toString("utf8");
          } catch (J) {
            L || m(J);
            return;
          }
          A(B, S);
        } else
          ++C, E.fetch(B, function(J, M) {
            if (--C, !!y) {
              if (J) {
                L ? C || m(null, E) : m(J);
                return;
              }
              A(B, M);
            }
          });
      }
    }
    var C = 0;
    d.isString(o) && (o = [o]);
    for (var j = 0, K; j < o.length; ++j)
      (K = E.resolvePath("", o[j])) && I(K);
    return v ? (E.resolveAll(), E) : (C || m(null, E), E);
  }, i.prototype.loadSync = function(o, p) {
    if (!d.isNode)
      throw Error("not supported");
    return this.load(o, p, t);
  }, i.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    if (this.deferred.length)
      throw Error("unresolvable extensions: " + this.deferred.map(function(o) {
        return "'extend " + o.extend + "' in " + o.parent.fullName;
      }).join(", "));
    return a.prototype.resolveAll.call(this);
  };
  var l = /^[A-Z]/;
  function s(u, o) {
    var p = o.parent.lookup(o.extend);
    if (p) {
      var y = new f(o.fullName, o.id, o.type, o.rule, void 0, o.options);
      return p.get(y.name) || (y.declaringField = o, o.extensionField = y, p.add(y)), !0;
    }
    return !1;
  }
  return i.prototype._handleAdd = function(o) {
    if (o instanceof f)
      /* an extension field (implies not part of a oneof) */
      o.extend !== void 0 && /* not already handled */
      !o.extensionField && (s(this, o) || this.deferred.push(o));
    else if (o instanceof h)
      l.test(o.name) && (o.parent[o.name] = o.values);
    else if (!(o instanceof c)) {
      if (o instanceof n)
        for (var p = 0; p < this.deferred.length; )
          s(this, this.deferred[p]) ? this.deferred.splice(p, 1) : ++p;
      for (var y = 0; y < /* initializes */
      o.nestedArray.length; ++y)
        this._handleAdd(o._nestedArray[y]);
      l.test(o.name) && (o.parent[o.name] = o);
    }
    (o instanceof n || o instanceof h || o instanceof f) && (this._fullyQualifiedObjects[o.fullName] = o);
  }, i.prototype._handleRemove = function(o) {
    if (o instanceof f) {
      if (
        /* an extension field */
        o.extend !== void 0
      )
        if (
          /* already handled */
          o.extensionField
        )
          o.extensionField.parent.remove(o.extensionField), o.extensionField = null;
        else {
          var p = this.deferred.indexOf(o);
          p > -1 && this.deferred.splice(p, 1);
        }
    } else if (o instanceof h)
      l.test(o.name) && delete o.parent[o.name];
    else if (o instanceof a) {
      for (var y = 0; y < /* initializes */
      o.nestedArray.length; ++y)
        this._handleRemove(o._nestedArray[y]);
      l.test(o.name) && delete o.parent[o.name];
    }
    delete this._fullyQualifiedObjects[o.fullName];
  }, i._configure = function(u, o, p) {
    n = u, e = o, r = p;
  }, root$1;
}
var hasRequiredUtil;
function requireUtil() {
  if (hasRequiredUtil) return util.exports;
  hasRequiredUtil = 1;
  var a = util.exports = requireMinimal(), f = requireRoots(), h, c;
  a.codegen = requireCodegen(), a.fetch = requireFetch(), a.path = requirePath(), a.fs = a.inquire("fs"), a.toArray = function(t) {
    if (t) {
      for (var l = Object.keys(t), s = new Array(l.length), u = 0; u < l.length; )
        s[u] = t[l[u++]];
      return s;
    }
    return [];
  }, a.toObject = function(t) {
    for (var l = {}, s = 0; s < t.length; ) {
      var u = t[s++], o = t[s++];
      o !== void 0 && (l[u] = o);
    }
    return l;
  };
  var d = /\\\\/g, n = /"/g;
  a.isReserved = function(t) {
    return /^(?:do|if|in|for|let|new|try|var|case|else|enum|eval|false|null|this|true|void|with|break|catch|class|const|super|throw|while|yield|delete|export|import|public|return|static|switch|typeof|default|extends|finally|package|private|continue|debugger|function|arguments|interface|protected|implements|instanceof)$/.test(t);
  }, a.safeProp = function(t) {
    return !/^[$\\w_]+$/.test(t) || a.isReserved(t) ? '["' + t.replace(d, "\\\\\\\\").replace(n, '\\\\"') + '"]' : "." + t;
  }, a.ucFirst = function(t) {
    return t.charAt(0).toUpperCase() + t.substring(1);
  };
  var e = /_([a-z])/g;
  a.camelCase = function(t) {
    return t.substring(0, 1) + t.substring(1).replace(e, function(l, s) {
      return s.toUpperCase();
    });
  }, a.compareFieldsById = function(t, l) {
    return t.id - l.id;
  }, a.decorateType = function(t, l) {
    if (t.$type)
      return l && t.$type.name !== l && (a.decorateRoot.remove(t.$type), t.$type.name = l, a.decorateRoot.add(t.$type)), t.$type;
    h || (h = requireType());
    var s = new h(l || t.name);
    return a.decorateRoot.add(s), s.ctor = t, Object.defineProperty(t, "$type", { value: s, enumerable: !1 }), Object.defineProperty(t.prototype, "$type", { value: s, enumerable: !1 }), s;
  };
  var r = 0;
  return a.decorateEnum = function(t) {
    if (t.$type)
      return t.$type;
    c || (c = require_enum());
    var l = new c("Enum" + r++, t);
    return a.decorateRoot.add(l), Object.defineProperty(t, "$type", { value: l, enumerable: !1 }), l;
  }, a.setProperty = function(t, l, s, u) {
    function o(p, y, E) {
      var v = y.shift();
      if (v === "__proto__" || v === "prototype")
        return p;
      if (y.length > 0)
        p[v] = o(p[v] || {}, y, E);
      else {
        var m = p[v];
        if (m && u)
          return p;
        m && (E = [].concat(m).concat(E)), p[v] = E;
      }
      return p;
    }
    if (typeof t != "object")
      throw TypeError("dst must be an object");
    if (!l)
      throw TypeError("path must be specified");
    return l = l.split("."), o(t, l, s);
  }, Object.defineProperty(a, "decorateRoot", {
    get: function() {
      return f.decorated || (f.decorated = new (requireRoot())());
    }
  }), util.exports;
}
var hasRequiredTypes;
function requireTypes() {
  return hasRequiredTypes || (hasRequiredTypes = 1, function(a) {
    var f = a, h = requireUtil(), c = [
      "double",
      // 0
      "float",
      // 1
      "int32",
      // 2
      "uint32",
      // 3
      "sint32",
      // 4
      "fixed32",
      // 5
      "sfixed32",
      // 6
      "int64",
      // 7
      "uint64",
      // 8
      "sint64",
      // 9
      "fixed64",
      // 10
      "sfixed64",
      // 11
      "bool",
      // 12
      "string",
      // 13
      "bytes"
      // 14
    ];
    function d(n, e) {
      var r = 0, i = {};
      for (e |= 0; r < n.length; ) i[c[r + e]] = n[r++];
      return i;
    }
    f.basic = d([
      /* double   */
      1,
      /* float    */
      5,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0,
      /* string   */
      2,
      /* bytes    */
      2
    ]), f.defaults = d([
      /* double   */
      0,
      /* float    */
      0,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      0,
      /* sfixed32 */
      0,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      0,
      /* sfixed64 */
      0,
      /* bool     */
      !1,
      /* string   */
      "",
      /* bytes    */
      h.emptyArray,
      /* message  */
      null
    ]), f.long = d([
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1
    ], 7), f.mapKey = d([
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0,
      /* string   */
      2
    ], 2), f.packed = d([
      /* double   */
      1,
      /* float    */
      5,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0
    ]);
  }(types)), types;
}
var field, hasRequiredField;
function requireField() {
  if (hasRequiredField) return field;
  hasRequiredField = 1, field = e;
  var a = requireObject();
  ((e.prototype = Object.create(a.prototype)).constructor = e).className = "Field";
  var f = require_enum(), h = requireTypes(), c = requireUtil(), d, n = /^required|optional|repeated$/;
  e.fromJSON = function(i, t) {
    var l = new e(i, t.id, t.type, t.rule, t.extend, t.options, t.comment);
    return t.edition && (l._edition = t.edition), l._defaultEdition = "proto3", l;
  };
  function e(r, i, t, l, s, u, o) {
    if (c.isObject(l) ? (o = s, u = l, l = s = void 0) : c.isObject(s) && (o = u, u = s, s = void 0), a.call(this, r, u), !c.isInteger(i) || i < 0)
      throw TypeError("id must be a non-negative integer");
    if (!c.isString(t))
      throw TypeError("type must be a string");
    if (l !== void 0 && !n.test(l = l.toString().toLowerCase()))
      throw TypeError("rule must be a string rule");
    if (s !== void 0 && !c.isString(s))
      throw TypeError("extend must be a string");
    l === "proto3_optional" && (l = "optional"), this.rule = l && l !== "optional" ? l : void 0, this.type = t, this.id = i, this.extend = s || void 0, this.repeated = l === "repeated", this.map = !1, this.message = null, this.partOf = null, this.typeDefault = null, this.defaultValue = null, this.long = c.Long ? h.long[t] !== void 0 : (
      /* istanbul ignore next */
      !1
    ), this.bytes = t === "bytes", this.resolvedType = null, this.extensionField = null, this.declaringField = null, this.comment = o;
  }
  return Object.defineProperty(e.prototype, "required", {
    get: function() {
      return this._features.field_presence === "LEGACY_REQUIRED";
    }
  }), Object.defineProperty(e.prototype, "optional", {
    get: function() {
      return !this.required;
    }
  }), Object.defineProperty(e.prototype, "delimited", {
    get: function() {
      return this.resolvedType instanceof d && this._features.message_encoding === "DELIMITED";
    }
  }), Object.defineProperty(e.prototype, "packed", {
    get: function() {
      return this._features.repeated_field_encoding === "PACKED";
    }
  }), Object.defineProperty(e.prototype, "hasPresence", {
    get: function() {
      return this.repeated || this.map ? !1 : this.partOf || // oneofs
      this.declaringField || this.extensionField || // extensions
      this._features.field_presence !== "IMPLICIT";
    }
  }), e.prototype.setOption = function(i, t, l) {
    return a.prototype.setOption.call(this, i, t, l);
  }, e.prototype.toJSON = function(i) {
    var t = i ? !!i.keepComments : !1;
    return c.toObject([
      "edition",
      this._editionToJSON(),
      "rule",
      this.rule !== "optional" && this.rule || void 0,
      "type",
      this.type,
      "id",
      this.id,
      "extend",
      this.extend,
      "options",
      this.options,
      "comment",
      t ? this.comment : void 0
    ]);
  }, e.prototype.resolve = function() {
    if (this.resolved)
      return this;
    if ((this.typeDefault = h.defaults[this.type]) === void 0 ? (this.resolvedType = (this.declaringField ? this.declaringField.parent : this.parent).lookupTypeOrEnum(this.type), this.resolvedType instanceof d ? this.typeDefault = null : this.typeDefault = this.resolvedType.values[Object.keys(this.resolvedType.values)[0]]) : this.options && this.options.proto3_optional && (this.typeDefault = null), this.options && this.options.default != null && (this.typeDefault = this.options.default, this.resolvedType instanceof f && typeof this.typeDefault == "string" && (this.typeDefault = this.resolvedType.values[this.typeDefault])), this.options && (this.options.packed !== void 0 && this.resolvedType && !(this.resolvedType instanceof f) && delete this.options.packed, Object.keys(this.options).length || (this.options = void 0)), this.long)
      this.typeDefault = c.Long.fromNumber(this.typeDefault, this.type.charAt(0) === "u"), Object.freeze && Object.freeze(this.typeDefault);
    else if (this.bytes && typeof this.typeDefault == "string") {
      var i;
      c.base64.test(this.typeDefault) ? c.base64.decode(this.typeDefault, i = c.newBuffer(c.base64.length(this.typeDefault)), 0) : c.utf8.write(this.typeDefault, i = c.newBuffer(c.utf8.length(this.typeDefault)), 0), this.typeDefault = i;
    }
    return this.map ? this.defaultValue = c.emptyObject : this.repeated ? this.defaultValue = c.emptyArray : this.defaultValue = this.typeDefault, this.parent instanceof d && (this.parent.ctor.prototype[this.name] = this.defaultValue), a.prototype.resolve.call(this);
  }, e.prototype._inferLegacyProtoFeatures = function(i) {
    if (i !== "proto2" && i !== "proto3")
      return {};
    var t = {};
    if (this.rule === "required" && (t.field_presence = "LEGACY_REQUIRED"), this.parent && h.defaults[this.type] === void 0) {
      var l = this.parent.get(this.type.split(".").pop());
      l && l instanceof d && l.group && (t.message_encoding = "DELIMITED");
    }
    return this.getOption("packed") === !0 ? t.repeated_field_encoding = "PACKED" : this.getOption("packed") === !1 && (t.repeated_field_encoding = "EXPANDED"), t;
  }, e.prototype._resolveFeatures = function(i) {
    return a.prototype._resolveFeatures.call(this, this._edition || i);
  }, e.d = function(i, t, l, s) {
    return typeof t == "function" ? t = c.decorateType(t).name : t && typeof t == "object" && (t = c.decorateEnum(t).name), function(o, p) {
      c.decorateType(o.constructor).add(new e(p, i, t, l, { default: s }));
    };
  }, e._configure = function(i) {
    d = i;
  }, field;
}
var oneof, hasRequiredOneof;
function requireOneof() {
  if (hasRequiredOneof) return oneof;
  hasRequiredOneof = 1, oneof = c;
  var a = requireObject();
  ((c.prototype = Object.create(a.prototype)).constructor = c).className = "OneOf";
  var f = requireField(), h = requireUtil();
  function c(n, e, r, i) {
    if (Array.isArray(e) || (r = e, e = void 0), a.call(this, n, r), !(e === void 0 || Array.isArray(e)))
      throw TypeError("fieldNames must be an Array");
    this.oneof = e || [], this.fieldsArray = [], this.comment = i;
  }
  c.fromJSON = function(e, r) {
    return new c(e, r.oneof, r.options, r.comment);
  }, c.prototype.toJSON = function(e) {
    var r = e ? !!e.keepComments : !1;
    return h.toObject([
      "options",
      this.options,
      "oneof",
      this.oneof,
      "comment",
      r ? this.comment : void 0
    ]);
  };
  function d(n) {
    if (n.parent)
      for (var e = 0; e < n.fieldsArray.length; ++e)
        n.fieldsArray[e].parent || n.parent.add(n.fieldsArray[e]);
  }
  return c.prototype.add = function(e) {
    if (!(e instanceof f))
      throw TypeError("field must be a Field");
    return e.parent && e.parent !== this.parent && e.parent.remove(e), this.oneof.push(e.name), this.fieldsArray.push(e), e.partOf = this, d(this), this;
  }, c.prototype.remove = function(e) {
    if (!(e instanceof f))
      throw TypeError("field must be a Field");
    var r = this.fieldsArray.indexOf(e);
    if (r < 0)
      throw Error(e + " is not a member of " + this);
    return this.fieldsArray.splice(r, 1), r = this.oneof.indexOf(e.name), r > -1 && this.oneof.splice(r, 1), e.partOf = null, this;
  }, c.prototype.onAdd = function(e) {
    a.prototype.onAdd.call(this, e);
    for (var r = this, i = 0; i < this.oneof.length; ++i) {
      var t = e.get(this.oneof[i]);
      t && !t.partOf && (t.partOf = r, r.fieldsArray.push(t));
    }
    d(this);
  }, c.prototype.onRemove = function(e) {
    for (var r = 0, i; r < this.fieldsArray.length; ++r)
      (i = this.fieldsArray[r]).parent && i.parent.remove(i);
    a.prototype.onRemove.call(this, e);
  }, Object.defineProperty(c.prototype, "isProto3Optional", {
    get: function() {
      if (this.fieldsArray == null || this.fieldsArray.length !== 1)
        return !1;
      var n = this.fieldsArray[0];
      return n.options != null && n.options.proto3_optional === !0;
    }
  }), c.d = function() {
    for (var e = new Array(arguments.length), r = 0; r < arguments.length; )
      e[r] = arguments[r++];
    return function(t, l) {
      h.decorateType(t.constructor).add(new c(l, e)), Object.defineProperty(t, l, {
        get: h.oneOfGetter(e),
        set: h.oneOfSetter(e)
      });
    };
  }, oneof;
}
var object, hasRequiredObject;
function requireObject() {
  if (hasRequiredObject) return object;
  hasRequiredObject = 1, object = r, r.className = "ReflectionObject";
  const a = requireOneof();
  var f = requireUtil(), h, c = { enum_type: "OPEN", field_presence: "EXPLICIT", json_format: "ALLOW", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "PACKED", utf8_validation: "VERIFY", enforce_naming_style: "STYLE2024", default_symbol_visibility: "EXPORT_TOP_LEVEL" }, d = { enum_type: "OPEN", field_presence: "EXPLICIT", json_format: "ALLOW", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "PACKED", utf8_validation: "VERIFY", enforce_naming_style: "STYLE_LEGACY", default_symbol_visibility: "EXPORT_ALL" }, n = { enum_type: "CLOSED", field_presence: "EXPLICIT", json_format: "LEGACY_BEST_EFFORT", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "EXPANDED", utf8_validation: "NONE", enforce_naming_style: "STYLE_LEGACY", default_symbol_visibility: "EXPORT_ALL" }, e = { enum_type: "OPEN", field_presence: "IMPLICIT", json_format: "ALLOW", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "PACKED", utf8_validation: "VERIFY", enforce_naming_style: "STYLE_LEGACY", default_symbol_visibility: "EXPORT_ALL" };
  function r(i, t) {
    if (!f.isString(i))
      throw TypeError("name must be a string");
    if (t && !f.isObject(t))
      throw TypeError("options must be an object");
    this.options = t, this.parsedOptions = null, this.name = i, this._edition = null, this._defaultEdition = "proto2", this._features = {}, this._featuresResolved = !1, this.parent = null, this.resolved = !1, this.comment = null, this.filename = null;
  }
  return Object.defineProperties(r.prototype, {
    /**
     * Reference to the root namespace.
     * @name ReflectionObject#root
     * @type {Root}
     * @readonly
     */
    root: {
      get: function() {
        for (var i = this; i.parent !== null; )
          i = i.parent;
        return i;
      }
    },
    /**
     * Full name including leading dot.
     * @name ReflectionObject#fullName
     * @type {string}
     * @readonly
     */
    fullName: {
      get: function() {
        for (var i = [this.name], t = this.parent; t; )
          i.unshift(t.name), t = t.parent;
        return i.join(".");
      }
    }
  }), r.prototype.toJSON = /* istanbul ignore next */
  function() {
    throw Error();
  }, r.prototype.onAdd = function(t) {
    this.parent && this.parent !== t && this.parent.remove(this), this.parent = t, this.resolved = !1;
    var l = t.root;
    l instanceof h && l._handleAdd(this);
  }, r.prototype.onRemove = function(t) {
    var l = t.root;
    l instanceof h && l._handleRemove(this), this.parent = null, this.resolved = !1;
  }, r.prototype.resolve = function() {
    return this.resolved ? this : (this.root instanceof h && (this.resolved = !0), this);
  }, r.prototype._resolveFeaturesRecursive = function(t) {
    return this._resolveFeatures(this._edition || t);
  }, r.prototype._resolveFeatures = function(t) {
    if (!this._featuresResolved) {
      var l = {};
      if (!t)
        throw new Error("Unknown edition for " + this.fullName);
      var s = Object.assign(
        this.options ? Object.assign({}, this.options.features) : {},
        this._inferLegacyProtoFeatures(t)
      );
      if (this._edition) {
        if (t === "proto2")
          l = Object.assign({}, n);
        else if (t === "proto3")
          l = Object.assign({}, e);
        else if (t === "2023")
          l = Object.assign({}, d);
        else if (t === "2024")
          l = Object.assign({}, c);
        else
          throw new Error("Unknown edition: " + t);
        this._features = Object.assign(l, s || {}), this._featuresResolved = !0;
        return;
      }
      if (this.partOf instanceof a) {
        var u = Object.assign({}, this.partOf._features);
        this._features = Object.assign(u, s || {});
      } else if (!this.declaringField) if (this.parent) {
        var o = Object.assign({}, this.parent._features);
        this._features = Object.assign(o, s || {});
      } else
        throw new Error("Unable to find a parent for " + this.fullName);
      this.extensionField && (this.extensionField._features = this._features), this._featuresResolved = !0;
    }
  }, r.prototype._inferLegacyProtoFeatures = function() {
    return {};
  }, r.prototype.getOption = function(t) {
    if (this.options)
      return this.options[t];
  }, r.prototype.setOption = function(t, l, s) {
    return this.options || (this.options = {}), /^features\\./.test(t) ? f.setProperty(this.options, t, l, s) : (!s || this.options[t] === void 0) && (this.getOption(t) !== l && (this.resolved = !1), this.options[t] = l), this;
  }, r.prototype.setParsedOption = function(t, l, s) {
    this.parsedOptions || (this.parsedOptions = []);
    var u = this.parsedOptions;
    if (s) {
      var o = u.find(function(E) {
        return Object.prototype.hasOwnProperty.call(E, t);
      });
      if (o) {
        var p = o[t];
        f.setProperty(p, s, l);
      } else
        o = {}, o[t] = f.setProperty({}, s, l), u.push(o);
    } else {
      var y = {};
      y[t] = l, u.push(y);
    }
    return this;
  }, r.prototype.setOptions = function(t, l) {
    if (t)
      for (var s = Object.keys(t), u = 0; u < s.length; ++u)
        this.setOption(s[u], t[s[u]], l);
    return this;
  }, r.prototype.toString = function() {
    var t = this.constructor.className, l = this.fullName;
    return l.length ? t + " " + l : t;
  }, r.prototype._editionToJSON = function() {
    if (!(!this._edition || this._edition === "proto3"))
      return this._edition;
  }, r._configure = function(i) {
    h = i;
  }, object;
}
var _enum, hasRequired_enum;
function require_enum() {
  if (hasRequired_enum) return _enum;
  hasRequired_enum = 1, _enum = c;
  var a = requireObject();
  ((c.prototype = Object.create(a.prototype)).constructor = c).className = "Enum";
  var f = requireNamespace(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (a.call(this, d, e), n && typeof n != "object")
      throw TypeError("values must be an object");
    if (this.valuesById = {}, this.values = Object.create(this.valuesById), this.comment = r, this.comments = i || {}, this.valuesOptions = t, this._valuesFeatures = {}, this.reserved = void 0, n)
      for (var l = Object.keys(n), s = 0; s < l.length; ++s)
        typeof n[l[s]] == "number" && (this.valuesById[this.values[l[s]] = n[l[s]]] = l[s]);
  }
  return c.prototype._resolveFeatures = function(n) {
    return n = this._edition || n, a.prototype._resolveFeatures.call(this, n), Object.keys(this.values).forEach((e) => {
      var r = Object.assign({}, this._features);
      this._valuesFeatures[e] = Object.assign(r, this.valuesOptions && this.valuesOptions[e] && this.valuesOptions[e].features);
    }), this;
  }, c.fromJSON = function(n, e) {
    var r = new c(n, e.values, e.options, e.comment, e.comments);
    return r.reserved = e.reserved, e.edition && (r._edition = e.edition), r._defaultEdition = "proto3", r;
  }, c.prototype.toJSON = function(n) {
    var e = n ? !!n.keepComments : !1;
    return h.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      this.options,
      "valuesOptions",
      this.valuesOptions,
      "values",
      this.values,
      "reserved",
      this.reserved && this.reserved.length ? this.reserved : void 0,
      "comment",
      e ? this.comment : void 0,
      "comments",
      e ? this.comments : void 0
    ]);
  }, c.prototype.add = function(n, e, r, i) {
    if (!h.isString(n))
      throw TypeError("name must be a string");
    if (!h.isInteger(e))
      throw TypeError("id must be an integer");
    if (this.values[n] !== void 0)
      throw Error("duplicate name '" + n + "' in " + this);
    if (this.isReservedId(e))
      throw Error("id " + e + " is reserved in " + this);
    if (this.isReservedName(n))
      throw Error("name '" + n + "' is reserved in " + this);
    if (this.valuesById[e] !== void 0) {
      if (!(this.options && this.options.allow_alias))
        throw Error("duplicate id " + e + " in " + this);
      this.values[n] = e;
    } else
      this.valuesById[this.values[n] = e] = n;
    return i && (this.valuesOptions === void 0 && (this.valuesOptions = {}), this.valuesOptions[n] = i || null), this.comments[n] = r || null, this;
  }, c.prototype.remove = function(n) {
    if (!h.isString(n))
      throw TypeError("name must be a string");
    var e = this.values[n];
    if (e == null)
      throw Error("name '" + n + "' does not exist in " + this);
    return delete this.valuesById[e], delete this.values[n], delete this.comments[n], this.valuesOptions && delete this.valuesOptions[n], this;
  }, c.prototype.isReservedId = function(n) {
    return f.isReservedId(this.reserved, n);
  }, c.prototype.isReservedName = function(n) {
    return f.isReservedName(this.reserved, n);
  }, _enum;
}
var encoder_1, hasRequiredEncoder;
function requireEncoder() {
  if (hasRequiredEncoder) return encoder_1;
  hasRequiredEncoder = 1, encoder_1 = d;
  var a = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n, e, r, i) {
    return e.delimited ? n("types[%i].encode(%s,w.uint32(%i)).uint32(%i)", r, i, (e.id << 3 | 3) >>> 0, (e.id << 3 | 4) >>> 0) : n("types[%i].encode(%s,w.uint32(%i).fork()).ldelim()", r, i, (e.id << 3 | 2) >>> 0);
  }
  function d(n) {
    for (var e = h.codegen(["m", "w"], n.name + "$encode")("if(!w)")("w=Writer.create()"), r, i, t = (
      /* initializes */
      n.fieldsArray.slice().sort(h.compareFieldsById)
    ), r = 0; r < t.length; ++r) {
      var l = t[r].resolve(), s = n._fieldsArray.indexOf(l), u = l.resolvedType instanceof a ? "int32" : l.type, o = f.basic[u];
      i = "m" + h.safeProp(l.name), l.map ? (e("if(%s!=null&&Object.hasOwnProperty.call(m,%j)){", i, l.name)("for(var ks=Object.keys(%s),i=0;i<ks.length;++i){", i)("w.uint32(%i).fork().uint32(%i).%s(ks[i])", (l.id << 3 | 2) >>> 0, 8 | f.mapKey[l.keyType], l.keyType), o === void 0 ? e("types[%i].encode(%s[ks[i]],w.uint32(18).fork()).ldelim().ldelim()", s, i) : e(".uint32(%i).%s(%s[ks[i]]).ldelim()", 16 | o, u, i), e("}")("}")) : l.repeated ? (e("if(%s!=null&&%s.length){", i, i), l.packed && f.packed[u] !== void 0 ? e("w.uint32(%i).fork()", (l.id << 3 | 2) >>> 0)("for(var i=0;i<%s.length;++i)", i)("w.%s(%s[i])", u, i)("w.ldelim()") : (e("for(var i=0;i<%s.length;++i)", i), o === void 0 ? c(e, l, s, i + "[i]") : e("w.uint32(%i).%s(%s[i])", (l.id << 3 | o) >>> 0, u, i)), e("}")) : (l.optional && e("if(%s!=null&&Object.hasOwnProperty.call(m,%j))", i, l.name), o === void 0 ? c(e, l, s, i) : e("w.uint32(%i).%s(%s)", (l.id << 3 | o) >>> 0, u, i));
    }
    return e("return w");
  }
  return encoder_1;
}
var hasRequiredIndexLight;
function requireIndexLight() {
  if (hasRequiredIndexLight) return indexLight.exports;
  hasRequiredIndexLight = 1;
  var a = indexLight.exports = requireIndexMinimal();
  a.build = "light";
  function f(c, d, n) {
    return typeof d == "function" ? (n = d, d = new a.Root()) : d || (d = new a.Root()), d.load(c, n);
  }
  a.load = f;
  function h(c, d) {
    return d || (d = new a.Root()), d.loadSync(c);
  }
  return a.loadSync = h, a.encoder = requireEncoder(), a.decoder = requireDecoder(), a.verifier = requireVerifier(), a.converter = requireConverter(), a.ReflectionObject = requireObject(), a.Namespace = requireNamespace(), a.Root = requireRoot(), a.Enum = require_enum(), a.Type = requireType(), a.Field = requireField(), a.OneOf = requireOneof(), a.MapField = requireMapfield(), a.Service = requireService(), a.Method = requireMethod(), a.Message = requireMessage(), a.wrappers = requireWrappers(), a.types = requireTypes(), a.util = requireUtil(), a.ReflectionObject._configure(a.Root), a.Namespace._configure(a.Type, a.Service, a.Enum), a.Root._configure(a.Type), a.Field._configure(a.Type), indexLight.exports;
}
var tokenize_1, hasRequiredTokenize;
function requireTokenize() {
  if (hasRequiredTokenize) return tokenize_1;
  hasRequiredTokenize = 1, tokenize_1 = l;
  var a = /[\\s{}=;:[\\],'"()<>]/g, f = /(?:"([^"\\\\]*(?:\\\\.[^"\\\\]*)*)")/g, h = /(?:'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)')/g, c = /^ *[*/]+ */, d = /^\\s*\\*?\\/*/, n = /\\n/g, e = /\\s/, r = /\\\\(.?)/g, i = {
    0: "\\0",
    r: "\\r",
    n: \`
\`,
    t: "	"
  };
  function t(s) {
    return s.replace(r, function(u, o) {
      switch (o) {
        case "\\\\":
        case "":
          return o;
        default:
          return i[o] || "";
      }
    });
  }
  l.unescape = t;
  function l(s, u) {
    s = s.toString();
    var o = 0, p = s.length, y = 1, E = 0, v = {}, m = [], _ = null;
    function A(k) {
      return Error("illegal " + k + " (line " + y + ")");
    }
    function I() {
      var k = _ === "'" ? h : f;
      k.lastIndex = o - 1;
      var P = k.exec(s);
      if (!P)
        throw A("string");
      return o = k.lastIndex, S(_), _ = null, t(P[1]);
    }
    function C(k) {
      return s.charAt(k);
    }
    function j(k, P, F) {
      var W = {
        type: s.charAt(k++),
        lineEmpty: !1,
        leading: F
      }, H;
      u ? H = 2 : H = 3;
      var D = k - H, $;
      do
        if (--D < 0 || ($ = s.charAt(D)) === \`
\`) {
          W.lineEmpty = !0;
          break;
        }
      while ($ === " " || $ === "	");
      for (var Y = s.substring(k, P).split(n), z = 0; z < Y.length; ++z)
        Y[z] = Y[z].replace(u ? d : c, "").trim();
      W.text = Y.join(\`
\`).trim(), v[y] = W, E = y;
    }
    function K(k) {
      var P = B(k), F = s.substring(k, P), W = /^\\s*\\/\\//.test(F);
      return W;
    }
    function B(k) {
      for (var P = k; P < p && C(P) !== \`
\`; )
        P++;
      return P;
    }
    function L() {
      if (m.length > 0)
        return m.shift();
      if (_)
        return I();
      var k, P, F, W, H, D = o === 0;
      do {
        if (o === p)
          return null;
        for (k = !1; e.test(F = C(o)); )
          if (F === \`
\` && (D = !0, ++y), ++o === p)
            return null;
        if (C(o) === "/") {
          if (++o === p)
            throw A("comment");
          if (C(o) === "/")
            if (u) {
              if (W = o, H = !1, K(o - 1)) {
                H = !0;
                do
                  if (o = B(o), o === p || (o++, !D))
                    break;
                while (K(o));
              } else
                o = Math.min(p, B(o) + 1);
              H && (j(W, o, D), D = !0), y++, k = !0;
            } else {
              for (H = C(W = o + 1) === "/"; C(++o) !== \`
\`; )
                if (o === p)
                  return null;
              ++o, H && (j(W, o - 1, D), D = !0), ++y, k = !0;
            }
          else if ((F = C(o)) === "*") {
            W = o + 1, H = u || C(W) === "*";
            do {
              if (F === \`
\` && ++y, ++o === p)
                throw A("comment");
              P = F, F = C(o);
            } while (P !== "*" || F !== "/");
            ++o, H && (j(W, o - 2, D), D = !0), k = !0;
          } else
            return "/";
        }
      } while (k);
      var $ = o;
      a.lastIndex = 0;
      var Y = a.test(C($++));
      if (!Y)
        for (; $ < p && !a.test(C($)); )
          ++$;
      var z = s.substring(o, o = $);
      return (z === '"' || z === "'") && (_ = z), z;
    }
    function S(k) {
      m.push(k);
    }
    function J() {
      if (!m.length) {
        var k = L();
        if (k === null)
          return null;
        S(k);
      }
      return m[0];
    }
    function M(k, P) {
      var F = J(), W = F === k;
      if (W)
        return L(), !0;
      if (!P)
        throw A("token '" + F + "', '" + k + "' expected");
      return !1;
    }
    function T(k) {
      var P = null, F;
      return k === void 0 ? (F = v[y - 1], delete v[y - 1], F && (u || F.type === "*" || F.lineEmpty) && (P = F.leading ? F.text : null)) : (E < k && J(), F = v[k], delete v[k], F && !F.lineEmpty && (u || F.type === "/") && (P = F.leading ? null : F.text)), P;
    }
    return Object.defineProperty({
      next: L,
      peek: J,
      push: S,
      skip: M,
      cmnt: T
    }, "line", {
      get: function() {
        return y;
      }
    });
  }
  return tokenize_1;
}
var parse_1, hasRequiredParse;
function requireParse() {
  if (hasRequiredParse) return parse_1;
  hasRequiredParse = 1, parse_1 = I, I.filename = null, I.defaults = { keepCase: !1 };
  var a = requireTokenize(), f = requireRoot(), h = requireType(), c = requireField(), d = requireMapfield(), n = requireOneof(), e = require_enum(), r = requireService(), i = requireMethod(), t = requireObject(), l = requireTypes(), s = requireUtil(), u = /^[1-9][0-9]*$/, o = /^-?[1-9][0-9]*$/, p = /^0[x][0-9a-fA-F]+$/, y = /^-?0[x][0-9a-fA-F]+$/, E = /^0[0-7]+$/, v = /^-?0[0-7]+$/, m = /^(?![eE])[0-9]*(?:\\.[0-9]*)?(?:[eE][+-]?[0-9]+)?$/, _ = /^[a-zA-Z_][a-zA-Z_0-9]*$/, A = /^(?:\\.?[a-zA-Z_][a-zA-Z_0-9]*)(?:\\.[a-zA-Z_][a-zA-Z_0-9]*)*$/;
  function I(C, j, K) {
    j instanceof f || (K = j, j = new f()), K || (K = I.defaults);
    var B = K.preferTrailingComment || !1, L = a(C, K.alternateCommentMode || !1), S = L.next, J = L.push, M = L.peek, T = L.skip, k = L.cmnt, P = !0, F, W, H, D = "proto2", $ = j, Y = [], z = {}, ae = K.keepCase ? function(R) {
      return R;
    } : s.camelCase;
    function he() {
      Y.forEach((R) => {
        R._edition = D, Object.keys(z).forEach((g) => {
          R.getOption(g) === void 0 && R.setOption(g, z[g], !0);
        });
      });
    }
    function N(R, g, O) {
      var b = I.filename;
      return O || (I.filename = null), Error("illegal " + (g || "token") + " '" + R + "' (" + (b ? b + ", " : "") + "line " + L.line + ")");
    }
    function ee() {
      var R = [], g;
      do {
        if ((g = S()) !== '"' && g !== "'")
          throw N(g);
        R.push(S()), T(g), g = M();
      } while (g === '"' || g === "'");
      return R.join("");
    }
    function ue(R) {
      var g = S();
      switch (g) {
        case "'":
        case '"':
          return J(g), ee();
        case "true":
        case "TRUE":
          return !0;
        case "false":
        case "FALSE":
          return !1;
      }
      try {
        return pe(
          g,
          /* insideTryCatch */
          !0
        );
      } catch {
        if (A.test(g))
          return g;
        throw N(g, "value");
      }
    }
    function re(R, g) {
      var O, b;
      do
        if (g && ((O = M()) === '"' || O === "'")) {
          var w = ee();
          if (R.push(w), D >= 2023)
            throw N(w, "id");
        } else
          try {
            R.push([b = te(S()), T("to", !0) ? te(S()) : b]);
          } catch (x) {
            if (g && A.test(O) && D >= 2023)
              R.push(O);
            else
              throw x;
          }
      while (T(",", !0));
      var q = { options: void 0 };
      q.setOption = function(x, G) {
        this.options === void 0 && (this.options = {}), this.options[x] = G;
      }, Z(
        q,
        function(G) {
          if (G === "option")
            Q(q, G), T(";");
          else
            throw N(G);
        },
        function() {
          se(q);
        }
      );
    }
    function pe(R, g) {
      var O = 1;
      switch (R.charAt(0) === "-" && (O = -1, R = R.substring(1)), R) {
        case "inf":
        case "INF":
        case "Inf":
          return O * (1 / 0);
        case "nan":
        case "NAN":
        case "Nan":
        case "NaN":
          return NaN;
        case "0":
          return 0;
      }
      if (u.test(R))
        return O * parseInt(R, 10);
      if (p.test(R))
        return O * parseInt(R, 16);
      if (E.test(R))
        return O * parseInt(R, 8);
      if (m.test(R))
        return O * parseFloat(R);
      throw N(R, "number", g);
    }
    function te(R, g) {
      switch (R) {
        case "max":
        case "MAX":
        case "Max":
          return 536870911;
        case "0":
          return 0;
      }
      if (!g && R.charAt(0) === "-")
        throw N(R, "id");
      if (o.test(R))
        return parseInt(R, 10);
      if (y.test(R))
        return parseInt(R, 16);
      if (v.test(R))
        return parseInt(R, 8);
      throw N(R, "id");
    }
    function ye() {
      if (F !== void 0)
        throw N("package");
      if (F = S(), !A.test(F))
        throw N(F, "name");
      $ = $.define(F), T(";");
    }
    function me() {
      var R = M(), g;
      switch (R) {
        case "option":
          if (D < "2024")
            throw N("option");
          S(), ee(), T(";");
          return;
        case "weak":
          g = H || (H = []), S();
          break;
        case "public":
          S();
        // eslint-disable-next-line no-fallthrough
        default:
          g = W || (W = []);
          break;
      }
      R = ee(), T(";"), g.push(R);
    }
    function ve() {
      if (T("="), D = ee(), D < 2023)
        throw N(D, "syntax");
      T(";");
    }
    function ge() {
      if (T("="), D = ee(), !["2023", "2024"].includes(D))
        throw N(D, "edition");
      T(";");
    }
    function ie(R, g) {
      switch (g) {
        case "option":
          return Q(R, g), T(";"), !0;
        case "message":
          return ne(R, g), !0;
        case "enum":
          return de(R, g), !0;
        case "export":
        case "local":
          return D < "2024" || (g = S(), g === "export" || g === "local") || g !== "message" && g !== "enum" ? !1 : ie(R, g);
        case "service":
          return be(R, g), !0;
        case "extend":
          return Se(R, g), !0;
      }
      return !1;
    }
    function Z(R, g, O) {
      var b = L.line;
      if (R && (typeof R.comment != "string" && (R.comment = k()), R.filename = I.filename), T("{", !0)) {
        for (var w; (w = S()) !== "}"; )
          g(w);
        T(";", !0);
      } else
        O && O(), T(";"), R && (typeof R.comment != "string" || B) && (R.comment = k(b) || R.comment);
    }
    function ne(R, g) {
      if (!_.test(g = S()))
        throw N(g, "type name");
      var O = new h(g);
      Z(O, function(w) {
        if (!ie(O, w))
          switch (w) {
            case "map":
              Ee(O);
              break;
            case "required":
              if (D !== "proto2")
                throw N(w);
            /* eslint-disable no-fallthrough */
            case "repeated":
              X(O, w);
              break;
            case "optional":
              if (D === "proto3")
                X(O, "proto3_optional");
              else {
                if (D !== "proto2")
                  throw N(w);
                X(O, "optional");
              }
              break;
            case "oneof":
              Re(O, w);
              break;
            case "extensions":
              re(O.extensions || (O.extensions = []));
              break;
            case "reserved":
              re(O.reserved || (O.reserved = []), !0);
              break;
            default:
              if (D === "proto2" || !A.test(w))
                throw N(w);
              J(w), X(O, "optional");
              break;
          }
      }), R.add(O), R === $ && Y.push(O);
    }
    function X(R, g, O) {
      var b = S();
      if (b === "group") {
        _e(R, g);
        return;
      }
      for (; b.endsWith(".") || M().startsWith("."); )
        b += S();
      if (!A.test(b))
        throw N(b, "type");
      var w = S();
      if (!_.test(w))
        throw N(w, "name");
      w = ae(w), T("=");
      var q = new c(w, te(S()), b, g, O);
      if (Z(q, function(U) {
        if (U === "option")
          Q(q, U), T(";");
        else
          throw N(U);
      }, function() {
        se(q);
      }), g === "proto3_optional") {
        var x = new n("_" + w);
        q.setOption("proto3_optional", !0), x.add(q), R.add(x);
      } else
        R.add(q);
      R === $ && Y.push(q);
    }
    function _e(R, g) {
      if (D >= 2023)
        throw N("group");
      var O = S();
      if (!_.test(O))
        throw N(O, "name");
      var b = s.lcFirst(O);
      O === b && (O = s.ucFirst(O)), T("=");
      var w = te(S()), q = new h(O);
      q.group = !0;
      var x = new c(b, w, O, g);
      x.filename = I.filename, Z(q, function(U) {
        switch (U) {
          case "option":
            Q(q, U), T(";");
            break;
          case "required":
          case "repeated":
            X(q, U);
            break;
          case "optional":
            D === "proto3" ? X(q, "proto3_optional") : X(q, "optional");
            break;
          case "message":
            ne(q, U);
            break;
          case "enum":
            de(q, U);
            break;
          case "reserved":
            re(q.reserved || (q.reserved = []), !0);
            break;
          case "export":
          case "local":
            if (D < "2024")
              throw N(U);
            switch (U = S(), U) {
              case "message":
                ne(q, U);
                break;
              case "enum":
                ne(q, U);
                break;
              default:
                throw N(U);
            }
            break;
          /* istanbul ignore next */
          default:
            throw N(U);
        }
      }), R.add(q).add(x);
    }
    function Ee(R) {
      T("<");
      var g = S();
      if (l.mapKey[g] === void 0)
        throw N(g, "type");
      T(",");
      var O = S();
      if (!A.test(O))
        throw N(O, "type");
      T(">");
      var b = S();
      if (!_.test(b))
        throw N(b, "name");
      T("=");
      var w = new d(ae(b), te(S()), g, O);
      Z(w, function(x) {
        if (x === "option")
          Q(w, x), T(";");
        else
          throw N(x);
      }, function() {
        se(w);
      }), R.add(w);
    }
    function Re(R, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var O = new n(ae(g));
      Z(O, function(w) {
        w === "option" ? (Q(O, w), T(";")) : (J(w), X(O, "optional"));
      }), R.add(O);
    }
    function de(R, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var O = new e(g);
      Z(O, function(w) {
        switch (w) {
          case "option":
            Q(O, w), T(";");
            break;
          case "reserved":
            re(O.reserved || (O.reserved = []), !0), O.reserved === void 0 && (O.reserved = []);
            break;
          default:
            Oe(O, w);
        }
      }), R.add(O), R === $ && Y.push(O);
    }
    function Oe(R, g) {
      if (!_.test(g))
        throw N(g, "name");
      T("=");
      var O = te(S(), !0), b = {
        options: void 0
      };
      b.getOption = function(w) {
        return this.options[w];
      }, b.setOption = function(w, q) {
        t.prototype.setOption.call(b, w, q);
      }, b.setParsedOption = function() {
      }, Z(b, function(q) {
        if (q === "option")
          Q(b, q), T(";");
        else
          throw N(q);
      }, function() {
        se(b);
      }), R.add(g, O, b.comment, b.parsedOptions || b.options);
    }
    function Q(R, g) {
      var O, b, w = !0;
      for (g === "option" && (g = S()); g !== "="; ) {
        if (g === "(") {
          var q = S();
          T(")"), g = "(" + q + ")";
        }
        if (w) {
          if (w = !1, g.includes(".") && !g.includes("(")) {
            var x = g.split(".");
            O = x[0] + ".", g = x[1];
            continue;
          }
          O = g;
        } else
          b = b ? b += g : g;
        g = S();
      }
      var G = b ? O.concat(b) : O, U = ce(R, G);
      b = b && b[0] === "." ? b.slice(1) : b, O = O && O[O.length - 1] === "." ? O.slice(0, -1) : O, Ae(R, O, U, b);
    }
    function ce(R, g) {
      if (T("{", !0)) {
        for (var O = {}; !T("}", !0); ) {
          if (!_.test(V = S()))
            throw N(V, "name");
          if (V === null)
            throw N(V, "end of input");
          var b, w = V;
          if (T(":", !0), M() === "{")
            b = ce(R, g + "." + V);
          else if (M() === "[") {
            b = [];
            var q;
            if (T("[", !0)) {
              do
                q = ue(), b.push(q);
              while (T(",", !0));
              T("]"), typeof q < "u" && fe(R, g + "." + V, q);
            }
          } else
            b = ue(), fe(R, g + "." + V, b);
          var x = O[w];
          x && (b = [].concat(x).concat(b)), O[w] = b, T(",", !0), T(";", !0);
        }
        return O;
      }
      var G = ue();
      return fe(R, g, G), G;
    }
    function fe(R, g, O) {
      if ($ === R && /^features\\./.test(g)) {
        z[g] = O;
        return;
      }
      R.setOption && R.setOption(g, O);
    }
    function Ae(R, g, O, b) {
      R.setParsedOption && R.setParsedOption(g, O, b);
    }
    function se(R) {
      if (T("[", !0)) {
        do
          Q(R, "option");
        while (T(",", !0));
        T("]");
      }
      return R;
    }
    function be(R, g) {
      if (!_.test(g = S()))
        throw N(g, "service name");
      var O = new r(g);
      Z(O, function(w) {
        if (!ie(O, w))
          if (w === "rpc")
            we(O, w);
          else
            throw N(w);
      }), R.add(O), R === $ && Y.push(O);
    }
    function we(R, g) {
      var O = k(), b = g;
      if (!_.test(g = S()))
        throw N(g, "name");
      var w = g, q, x, G, U;
      if (T("("), T("stream", !0) && (x = !0), !A.test(g = S()) || (q = g, T(")"), T("returns"), T("("), T("stream", !0) && (U = !0), !A.test(g = S())))
        throw N(g);
      G = g, T(")");
      var oe = new i(w, b, q, G, x, U);
      oe.comment = O, Z(oe, function(le) {
        if (le === "option")
          Q(oe, le), T(";");
        else
          throw N(le);
      }), R.add(oe);
    }
    function Se(R, g) {
      if (!A.test(g = S()))
        throw N(g, "reference");
      var O = g;
      Z(null, function(w) {
        switch (w) {
          case "required":
          case "repeated":
            X(R, w, O);
            break;
          case "optional":
            D === "proto3" ? X(R, "proto3_optional", O) : X(R, "optional", O);
            break;
          default:
            if (D === "proto2" || !A.test(w))
              throw N(w);
            J(w), X(R, "optional", O);
            break;
        }
      });
    }
    for (var V; (V = S()) !== null; )
      switch (V) {
        case "package":
          if (!P)
            throw N(V);
          ye();
          break;
        case "import":
          if (!P)
            throw N(V);
          me();
          break;
        case "syntax":
          if (!P)
            throw N(V);
          ve();
          break;
        case "edition":
          if (!P)
            throw N(V);
          ge();
          break;
        case "option":
          Q($, V), T(";", !0);
          break;
        default:
          if (ie($, V)) {
            P = !1;
            continue;
          }
          throw N(V);
      }
    return he(), I.filename = null, {
      package: F,
      imports: W,
      weakImports: H,
      root: j
    };
  }
  return parse_1;
}
var common_1, hasRequiredCommon;
function requireCommon() {
  if (hasRequiredCommon) return common_1;
  hasRequiredCommon = 1, common_1 = f;
  var a = /\\/|\\./;
  function f(c, d) {
    a.test(c) || (c = "google/protobuf/" + c + ".proto", d = { nested: { google: { nested: { protobuf: { nested: d } } } } }), f[c] = d;
  }
  f("any", {
    /**
     * Properties of a google.protobuf.Any message.
     * @interface IAny
     * @type {Object}
     * @property {string} [typeUrl]
     * @property {Uint8Array} [bytes]
     * @memberof common
     */
    Any: {
      fields: {
        type_url: {
          type: "string",
          id: 1
        },
        value: {
          type: "bytes",
          id: 2
        }
      }
    }
  });
  var h;
  return f("duration", {
    /**
     * Properties of a google.protobuf.Duration message.
     * @interface IDuration
     * @type {Object}
     * @property {number|Long} [seconds]
     * @property {number} [nanos]
     * @memberof common
     */
    Duration: h = {
      fields: {
        seconds: {
          type: "int64",
          id: 1
        },
        nanos: {
          type: "int32",
          id: 2
        }
      }
    }
  }), f("timestamp", {
    /**
     * Properties of a google.protobuf.Timestamp message.
     * @interface ITimestamp
     * @type {Object}
     * @property {number|Long} [seconds]
     * @property {number} [nanos]
     * @memberof common
     */
    Timestamp: h
  }), f("empty", {
    /**
     * Properties of a google.protobuf.Empty message.
     * @interface IEmpty
     * @memberof common
     */
    Empty: {
      fields: {}
    }
  }), f("struct", {
    /**
     * Properties of a google.protobuf.Struct message.
     * @interface IStruct
     * @type {Object}
     * @property {Object.<string,IValue>} [fields]
     * @memberof common
     */
    Struct: {
      fields: {
        fields: {
          keyType: "string",
          type: "Value",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.Value message.
     * @interface IValue
     * @type {Object}
     * @property {string} [kind]
     * @property {0} [nullValue]
     * @property {number} [numberValue]
     * @property {string} [stringValue]
     * @property {boolean} [boolValue]
     * @property {IStruct} [structValue]
     * @property {IListValue} [listValue]
     * @memberof common
     */
    Value: {
      oneofs: {
        kind: {
          oneof: [
            "nullValue",
            "numberValue",
            "stringValue",
            "boolValue",
            "structValue",
            "listValue"
          ]
        }
      },
      fields: {
        nullValue: {
          type: "NullValue",
          id: 1
        },
        numberValue: {
          type: "double",
          id: 2
        },
        stringValue: {
          type: "string",
          id: 3
        },
        boolValue: {
          type: "bool",
          id: 4
        },
        structValue: {
          type: "Struct",
          id: 5
        },
        listValue: {
          type: "ListValue",
          id: 6
        }
      }
    },
    NullValue: {
      values: {
        NULL_VALUE: 0
      }
    },
    /**
     * Properties of a google.protobuf.ListValue message.
     * @interface IListValue
     * @type {Object}
     * @property {Array.<IValue>} [values]
     * @memberof common
     */
    ListValue: {
      fields: {
        values: {
          rule: "repeated",
          type: "Value",
          id: 1
        }
      }
    }
  }), f("wrappers", {
    /**
     * Properties of a google.protobuf.DoubleValue message.
     * @interface IDoubleValue
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    DoubleValue: {
      fields: {
        value: {
          type: "double",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.FloatValue message.
     * @interface IFloatValue
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    FloatValue: {
      fields: {
        value: {
          type: "float",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.Int64Value message.
     * @interface IInt64Value
     * @type {Object}
     * @property {number|Long} [value]
     * @memberof common
     */
    Int64Value: {
      fields: {
        value: {
          type: "int64",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.UInt64Value message.
     * @interface IUInt64Value
     * @type {Object}
     * @property {number|Long} [value]
     * @memberof common
     */
    UInt64Value: {
      fields: {
        value: {
          type: "uint64",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.Int32Value message.
     * @interface IInt32Value
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    Int32Value: {
      fields: {
        value: {
          type: "int32",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.UInt32Value message.
     * @interface IUInt32Value
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    UInt32Value: {
      fields: {
        value: {
          type: "uint32",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.BoolValue message.
     * @interface IBoolValue
     * @type {Object}
     * @property {boolean} [value]
     * @memberof common
     */
    BoolValue: {
      fields: {
        value: {
          type: "bool",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.StringValue message.
     * @interface IStringValue
     * @type {Object}
     * @property {string} [value]
     * @memberof common
     */
    StringValue: {
      fields: {
        value: {
          type: "string",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.BytesValue message.
     * @interface IBytesValue
     * @type {Object}
     * @property {Uint8Array} [value]
     * @memberof common
     */
    BytesValue: {
      fields: {
        value: {
          type: "bytes",
          id: 1
        }
      }
    }
  }), f("field_mask", {
    /**
     * Properties of a google.protobuf.FieldMask message.
     * @interface IDoubleValue
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    FieldMask: {
      fields: {
        paths: {
          rule: "repeated",
          type: "string",
          id: 1
        }
      }
    }
  }), f.get = function(d) {
    return f[d] || null;
  }, common_1;
}
var hasRequiredSrc;
function requireSrc() {
  if (hasRequiredSrc) return src.exports;
  hasRequiredSrc = 1;
  var a = src.exports = requireIndexLight();
  return a.build = "full", a.tokenize = requireTokenize(), a.parse = requireParse(), a.common = requireCommon(), a.Root._configure(a.Type, a.parse, a.common), src.exports;
}
var protobufjs, hasRequiredProtobufjs;
function requireProtobufjs() {
  return hasRequiredProtobufjs || (hasRequiredProtobufjs = 1, protobufjs = requireSrc()), protobufjs;
}
var protobufjsExports = requireProtobufjs(), StateStreamErrorCode = /* @__PURE__ */ ((a) => (a.CONNECTION_FAILED = "CONNECTION_FAILED", a.RECONNECT_FAILED = "RECONNECT_FAILED", a.CONNECTION_LOST = "CONNECTION_LOST", a.CONNECTION_TIMEOUT = "CONNECTION_TIMEOUT", a.AUTH_FAILED = "AUTH_FAILED", a.AUTH_REFRESH_FAILED = "AUTH_REFRESH_FAILED", a.DEVICE_ERROR = "DEVICE_ERROR", a.DECODE_ERROR = "DECODE_ERROR", a.FRAME_PROCESS_ERROR = "FRAME_PROCESS_ERROR", a.STREAM_ALREADY_STARTED = "STREAM_ALREADY_STARTED", a.WORKER_INIT_FAILED = "WORKER_INIT_FAILED", a.UNKNOWN_ERROR = "UNKNOWN_ERROR", a))(StateStreamErrorCode || {}), ConnectionStatus = /* @__PURE__ */ ((a) => (a.DISCONNECTED = "DISCONNECTED", a.CONNECTING = "CONNECTING", a.CONNECTED = "CONNECTED", a.RECONNECTING = "RECONNECTING", a))(ConnectionStatus || {}), AuthStatus = /* @__PURE__ */ ((a) => (a.UNAUTHENTICATED = "UNAUTHENTICATED", a.AUTHENTICATING = "AUTHENTICATING", a.AUTHENTICATED = "AUTHENTICATED", a.REAUTHENTICATING = "REAUTHENTICATING", a.FAILED = "FAILED", a))(AuthStatus || {});
const DEFAULT_MAX_RECONNECT_ATTEMPTS = 5, DEFAULT_MAX_AUTH_ATTEMPTS = 5, DEFAULT_DELAY_RECONNECT = 500, nested = { BSB_State: { nested: { StateUpdate: { oneofs: { state: { oneof: ["deviceName", "power", "brightness", "audioVolume", "wifi", "updateState", "updateCheck", "timezone", "matter", "frame", "input", "timer", "ble", "autoUpdateState"] } }, fields: { deviceName: { type: "BSB_State.DeviceName", id: 1 }, power: { type: "BSB_State.Power", id: 2 }, brightness: { type: "BSB_State.Brightness", id: 3 }, audioVolume: { type: "BSB_State.AudioVolume", id: 4 }, wifi: { type: "BSB_State.Wifi", id: 5 }, updateState: { type: "BSB_Update.UpdateState", id: 6 }, updateCheck: { type: "BSB_Update.CheckState", id: 7 }, timezone: { type: "BSB_State.Timezone", id: 8 }, matter: { type: "BSB_State.Matter", id: 9 }, frame: { type: "BSB_Frame.Frame", id: 10 }, input: { type: "BSB_Input.InputEvent", id: 11 }, timer: { type: "BSB_Timer.Timer", id: 12 }, ble: { type: "BSB_State.Ble.Ble", id: 13 }, autoUpdateState: { type: "BSB_Update.AutoUpdateState", id: 14 } } }, State: { oneofs: { _error: { oneof: ["error"] } }, fields: { timestamp: { type: "fixed64", id: 1 }, updates: { rule: "repeated", type: "StateUpdate", id: 2 }, error: { type: "BSB_Error.Error", id: 3, options: { proto3_optional: !0 } } } }, DeviceName: { fields: { name: { type: "string", id: 1 } } }, BrightnessAutomatic: { fields: {} }, BrightnessManual: { fields: { brightness: { type: "uint32", id: 1 } } }, Brightness: { oneofs: { setting: { oneof: ["automatic", "manual"] } }, fields: { automatic: { type: "BrightnessAutomatic", id: 1 }, manual: { type: "BrightnessManual", id: 2 }, actualBrightness: { type: "uint32", id: 3 } } }, BatteryStatus: { values: { DISCHARGING: 0, CHARGING: 1, CHARGED: 2 } }, UnknownPowerState: { fields: {} }, PowerState: { fields: { batteryStatus: { type: "BatteryStatus", id: 1 }, batteryChargePercent: { type: "uint32", id: 2 }, batteryVoltageMv: { type: "uint32", id: 3 }, batteryCurrentMa: { type: "sint32", id: 4 }, usbVoltageMv: { type: "uint32", id: 5 } } }, Power: { oneofs: { state: { oneof: ["unknown", "known"] } }, fields: { unknown: { type: "UnknownPowerState", id: 1 }, known: { type: "PowerState", id: 2 } } }, AudioVolume: { fields: { volume: { type: "uint32", id: 1 } } }, WifiConnectionStatus: { values: { CONNECTED: 0, CONNECTING: 1, DISCONNECTING: 2, RECONNECTING: 3 } }, WifiSecurity: { values: { UNKNOWN: 0, OPEN: 1, WPA: 2, WPA2: 3, WEP: 4, WPA_WPA2: 5, WPA3: 6, WPA2_WPA3: 7 } }, IpConfigurationMethod: { values: { DHCP: 0, STATIC: 1 } }, IpProtocol: { values: { IPV4: 0, IPV6: 1 } }, WifiStateUnknown: { fields: {} }, WifiStateDisconnected: { fields: {} }, WifiStateConnected: { fields: { status: { type: "WifiConnectionStatus", id: 1 }, ssid: { type: "string", id: 2 }, bssid: { type: "string", id: 3 }, channel: { type: "uint32", id: 4 }, rssi: { type: "sint32", id: 5 }, security: { type: "WifiSecurity", id: 6 } } }, IpAddress: { fields: { protocol: { type: "IpProtocol", id: 1 }, method: { type: "IpConfigurationMethod", id: 2 }, address: { type: "string", id: 3 }, gateway: { type: "string", id: 4 }, netmask: { type: "string", id: 5 } } }, Wifi: { oneofs: { wifiState: { oneof: ["unknown", "disconnected", "connected"] } }, fields: { unknown: { type: "WifiStateUnknown", id: 1 }, disconnected: { type: "WifiStateDisconnected", id: 2 }, connected: { type: "WifiStateConnected", id: 3 }, ipAddresses: { rule: "repeated", type: "IpAddress", id: 4 } } }, Timezone: { fields: { name: { type: "string", id: 1 }, offset: { type: "sint32", id: 2 }, abbr: { type: "string", id: 3 } } }, MatterCommissioningStatus: { values: { NEVER_STARTED: 0, STARTED: 1, COMPLETED_SUCCESSFULLY: 2, FAILED: 3 } }, MatterCommissioningState: { fields: { status: { type: "MatterCommissioningStatus", id: 1 }, timestamp: { type: "fixed64", id: 2 } } }, Matter: { fields: { fabricCount: { type: "uint32", id: 1 }, state: { type: "MatterCommissioningState", id: 2 } } }, Ble: { nested: { ServiceStatus: { values: { RESET: 0, INITIALIZATION: 1, READY: 2, ADVERTISING: 3, CONNECTABLE: 4, CONNECTED: 5, ERROR: 6 } }, Ble: { oneofs: { _remoteAddress: { oneof: ["remoteAddress"] } }, fields: { status: { type: "ServiceStatus", id: 1 }, remoteAddress: { type: "string", id: 2, options: { proto3_optional: !0 } } } } } } } }, BSB_Update: { nested: { UpdateEvent: { values: { SESSION_START: 0, SESSION_STOP: 1, ACTION_BEGIN: 2, ACTION_DONE: 3, DETAIL_CHANGE: 4, ACTION_PROGRESS: 5, EVENT_NONE: 6 } }, UpdateAction: { values: { DOWNLOAD: 0, SHA_VERIFICATION: 1, UNPACK: 2, INSTALLATION_PREPARE: 3, INSTALLATION_APPLY: 4, ACTION_NONE: 5 } }, UpdateStatus: { values: { OK: 0, BATTERY_LOW: 1, BUSY: 2, DOWNLOAD_FAILURE: 3, DOWNLOAD_ABORT: 4, SHA_MISMATCH: 5, UNPACK_CREATE_STAGING_DIRECTORY_FAILURE: 6, UNPACK_ARCHIVE_OPEN_FAILURE: 7, UNPACK_ARCHIVE_UNPACK_FAILURE: 8, INSTALLATION_PREPARE_MANIFEST_NOT_FOUND: 9, INSTALLATION_PREPARE_MANIFEST_INVALID: 10, INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE: 11, INSTALLATION_PREPARE_POINTER_SETUP_FAILURE: 12, UNKNOWN_FAILURE: 13 } }, CheckError: { values: { NOT_AVAILABLE: 0, FAILURE: 1, IDLE: 2 } }, UpdateAvailable: { fields: { version: { type: "string", id: 1 } } }, UpdateUnavailable: { fields: { reason: { type: "CheckError", id: 1 } } }, UpdateState: { fields: { event: { type: "UpdateEvent", id: 1 }, action: { type: "UpdateAction", id: 2 }, status: { type: "UpdateStatus", id: 3 } } }, CheckState: { oneofs: { status: { oneof: ["available", "unavailable"] } }, fields: { available: { type: "UpdateAvailable", id: 1 }, unavailable: { type: "UpdateUnavailable", id: 2 } } }, AutoUpdateInterval: { fields: { start: { type: "uint32", id: 1 }, end: { type: "uint32", id: 2 } } }, AutoUpdateState: { fields: { enabled: { type: "bool", id: 1 }, interval: { type: "AutoUpdateInterval", id: 2 } } } } }, BSB_Frame: { nested: { Encoding: { values: { PLAIN: 0, RUN_LENGTH: 1, DEFLATE: 2, DEFLATE_RUN_LENGTH: 3 } }, PixelFormat: { values: { RGB888: 0, L8: 1, L4: 2 } }, Screen: { values: { FRONT: 0, BACK: 1 } }, Frame: { fields: { screen: { type: "Screen", id: 1 }, width: { type: "uint32", id: 2 }, height: { type: "uint32", id: 3 }, encoding: { type: "Encoding", id: 4 }, pixelFormat: { type: "PixelFormat", id: 5 }, data: { type: "bytes", id: 6 } } } } }, BSB_Timer: { nested: { Timer: { fields: { json: { type: "BSB_Util.Json", id: 1 } } } } }, BSB_Util: { nested: { Compression: { values: { PLAIN: 0, GZIP: 1 } }, Json: { fields: { compression: { type: "Compression", id: 1 }, data: { type: "bytes", id: 2 } } } } }, BSB_Input: { nested: { Button: { values: { OK: 0, BACK: 1, START: 2 } }, ButtonAction: { values: { PRESS: 0, RELEASE: 1 } }, SwitchPosition: { values: { BUSY: 0, CUSTOM: 1, OFF: 2, APPS: 3, SETTINGS: 4 } }, ButtonEvent: { fields: { button: { type: "Button", id: 1 }, action: { type: "ButtonAction", id: 2 } } }, SwitchEvent: { fields: { position: { type: "SwitchPosition", id: 1 } } }, EncoderEvent: { fields: { delta: { type: "sint32", id: 1 } } }, InputEvent: { oneofs: { event: { oneof: ["buttonEvent", "switchEvent", "encoderEvent"] } }, fields: { buttonEvent: { type: "ButtonEvent", id: 1 }, switchEvent: { type: "SwitchEvent", id: 2 }, encoderEvent: { type: "EncoderEvent", id: 3 } } } } }, BSB_Error: { nested: { Cause: { values: { RESOURCE_LIMIT: 0 } }, Severity: { values: { FATAL: 0, ERROR: 1, WARNING: 2 } }, Error: { fields: { cause: { type: "Cause", id: 1 }, severity: { type: "Severity", id: 2 } } } } } };
var bundle = {
  nested
};
function decompressRLE(a, f) {
  const h = [];
  for (let c = 0; c < a.length; ) {
    const d = a[c++];
    if (d === void 0) break;
    const n = d & 127;
    if (!n)
      continue;
    if (d & 128) {
      const r = n * f, i = a.subarray(c, c + r);
      for (let t = 0; t < i.length; t++)
        h.push(i[t]);
      c += r;
      continue;
    }
    const e = a.subarray(c, c + f);
    c += f;
    for (let r = 0; r < n; r++)
      for (let i = 0; i < f; i++)
        h.push(e[i]);
  }
  return new Uint8Array(h);
}
async function decompressDeflate(a) {
  if (typeof DecompressionStream > "u")
    throw new Error("DecompressionStream is not supported in this environment.");
  try {
    const f = new DecompressionStream("deflate"), h = f.writable.getWriter();
    h.write(a), h.close();
    const d = await new Response(f.readable).arrayBuffer();
    return new Uint8Array(d);
  } catch (f) {
    throw new Error(\`Deflate decompression failed: \${f instanceof Error ? f.message : String(f)}\`);
  }
}
function convertL4toRGBA(a, f, h) {
  const c = new Uint8ClampedArray(f * h * 4);
  let d = 0;
  for (let n = 0; n < a.length; n++) {
    const e = a[n], r = (e & 15) * 17, i = (e >> 4 & 15) * 17, t = [r, i];
    for (const l of t)
      if (d < f * h) {
        const s = d * 4;
        c[s] = l, c[s + 1] = l, c[s + 2] = l, c[s + 3] = 255, d++;
      }
  }
  return c;
}
function convertL8toRGBA(a, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = Math.min(a.length, f * h);
  for (let n = 0; n < d; n++) {
    const e = a[n], r = n * 4;
    c[r] = e, c[r + 1] = e, c[r + 2] = e, c[r + 3] = 255;
  }
  return c;
}
function convertRGB888toRGBA(a, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = f * h;
  for (let n = 0; n < d; n++) {
    const e = n * 3, r = n * 4;
    e + 2 < a.length && (c[r] = a[e + 2], c[r + 1] = a[e + 1], c[r + 2] = a[e], c[r + 3] = 255);
  }
  return c;
}
async function processFrame(a) {
  if (!a.data || !a.width || !a.height)
    return null;
  let f = a.data;
  const h = a.pixelFormat === 0 ? 3 : 1;
  switch (a.encoding) {
    case 1:
      f = decompressRLE(f, h);
      break;
    case 2:
      f = await decompressDeflate(f);
      break;
    case 3:
      f = await decompressDeflate(f), f = decompressRLE(f, h);
      break;
  }
  switch (a.pixelFormat) {
    case 2:
      return convertL4toRGBA(f, a.width, a.height);
    case 1:
      return convertL8toRGBA(f, a.width, a.height);
    case 0:
      return convertRGB888toRGBA(f, a.width, a.height);
    default:
      return new Uint8ClampedArray(a.width * a.height * 4);
  }
}
const root = protobufjsExports.Root.fromJSON(bundle), StateType = root.lookupType("BSB_State.State"), AUTH_CODE = 3e3;
let maxAuthAttempts = DEFAULT_MAX_AUTH_ATTEMPTS, maxReconnectAttempts = DEFAULT_MAX_RECONNECT_ATTEMPTS, reconnectDelay = DEFAULT_DELAY_RECONNECT, socket = null, isBinaryMode = !0, currentMode = "local", currentToken, currentAddr = "", retryCount = 0, authRetryCount = 0, isAuthReported = !1;
const activePorts = /* @__PURE__ */ new Set(), subscriptions = /* @__PURE__ */ new Map();
let processingQueue = Promise.resolve();
function broadcast(a) {
  for (const f of activePorts)
    f.postMessage(a);
}
function sendAuth() {
  currentMode === "remote" && currentToken && (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && (broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATING }), socket.send(JSON.stringify({ token: currentToken })));
}
function sendSubscriptions() {
  (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && subscriptions.size > 0 && socket.send(
    JSON.stringify({
      subscribe: Array.from(subscriptions.keys())
    })
  );
}
function stopSocket() {
  socket && (socket.onopen = null, socket.onmessage = null, socket.onerror = null, socket.onclose = null, socket.close(), socket = null);
}
function stopAndCleanup() {
  stopSocket(), subscriptions.clear(), activePorts.clear(), retryCount = 0, authRetryCount = 0, isAuthReported = !1;
}
function connect(a, f, h = !0, c = "local") {
  stopSocket(), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.CONNECTING }), currentAddr = a, isBinaryMode = h, currentMode = c, currentToken = f, isAuthReported = !1;
  const d = new URL(a);
  socket = new WebSocket(d.toString()), socket.binaryType = "arraybuffer", socket.onopen = () => {
    broadcast({ type: "CONNECTED" }), currentMode === "local" && (socket == null || socket.send(JSON.stringify({ enable: !0 }))), sendAuth(), retryCount = 0, authRetryCount = 0, console.log("[Worker] Connection stable. All retry counters reset."), currentMode === "remote" && subscriptions.size > 0 && sendSubscriptions(), currentMode === "local" && broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED });
  }, socket.onmessage = (n) => {
    processingQueue = processingQueue.then(async () => {
      try {
        let e = null, r = "", i;
        if (isBinaryMode)
          n.data instanceof ArrayBuffer && (e = new Uint8Array(n.data), r = e);
        else {
          const t = JSON.parse(n.data);
          i = t.bar_id || t.barId, r = n.data, t.state && (typeof t.state == "string" ? e = Uint8Array.from(atob(t.state), (l) => l.charCodeAt(0)) : e = new Uint8Array(t.state)), currentMode === "remote" && !isAuthReported && (isAuthReported = !0, broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED }));
        }
        if (r && broadcast({ type: "RAW_DATA", data: r }), e) {
          const t = StateType.decode(e), l = StateType.toObject(t, {
            longs: Number,
            bytes: Uint8Array,
            enums: Number,
            defaults: !0
          });
          if (l.error) {
            const { cause: s, severity: u } = l.error;
            if (s != null && u != null) {
              const o = root.lookupEnum("BSB_Error.Cause"), p = root.lookupEnum("BSB_Error.Severity"), y = o.valuesById[s] || "UNKNOWN", E = p.valuesById[u] || "UNKNOWN";
              if (broadcast({
                type: "ERROR",
                code: StateStreamErrorCode.DEVICE_ERROR,
                message: \`Server reported \${E}: \${y}\`,
                data: l.error
              }), u === p.values.FATAL) {
                stopAndCleanup();
                return;
              }
              if (u === p.values.ERROR)
                return;
            } else
              broadcast({
                type: "ERROR",
                code: StateStreamErrorCode.DEVICE_ERROR,
                message: "Server reported an unspecified application error",
                data: l.error
              });
          }
          if (l.updates)
            for (const s of l.updates) {
              const u = s.frame;
              if (u && u.data)
                try {
                  const o = await processFrame(u);
                  o && (u.data = o);
                } catch (o) {
                  broadcast({
                    type: "ERROR",
                    code: StateStreamErrorCode.FRAME_PROCESS_ERROR,
                    message: o instanceof Error ? o.message : String(o),
                    data: u.data
                  });
                }
            }
          broadcast(currentMode === "remote" && i ? {
            type: "DATA",
            data: { bar_id: i, state: l }
          } : { type: "DATA", data: l });
        }
      } catch (e) {
        broadcast({
          type: "ERROR",
          code: StateStreamErrorCode.DECODE_ERROR,
          message: \`Decode error: \${String(e)}\`,
          data: n.data
        });
      }
    }).catch(console.error);
  }, socket.onerror = () => {
    broadcast({ type: "ERROR", code: StateStreamErrorCode.CONNECTION_FAILED, message: "WebSocket connection error" });
  }, socket.onclose = (n) => {
    if (console.log("[Worker] Socket closed:", n), !socket || activePorts.size === 0) {
      console.log("[Worker] Connection closed or no active ports. No retries.");
      return;
    }
    if (n.code === AUTH_CODE && currentMode === "remote") {
      authRetryCount < maxAuthAttempts ? (authRetryCount++, console.warn(\`[Worker] Auth failed (3000). Requesting new token... (Attempt \${authRetryCount}/\${maxAuthAttempts})\`), broadcast({ type: "TOKEN_EXPIRED" }), broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.REAUTHENTICATING, authAttempts: authRetryCount })) : (broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.FAILED }), broadcast({
        type: "ERROR",
        code: StateStreamErrorCode.AUTH_FAILED,
        message: \`Maximum authentication attempts (\${maxAuthAttempts}) reached. Please log in again.\`
      }));
      return;
    }
    if (n.code !== 1e3) {
      if (retryCount < maxReconnectAttempts) {
        retryCount++;
        let e = Math.min(1e3 * retryCount, 5e3);
        reconnectDelay && (e = reconnectDelay), console.log(\`[Worker] Reconnecting (network code: \${n.code}) in \${e}ms... (Attempt \${retryCount}/\${maxReconnectAttempts})\`), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.RECONNECTING, connectionAttempts: retryCount }), setTimeout(() => {
          activePorts.size > 0 && socket && connect(currentAddr, currentToken, isBinaryMode, currentMode);
        }, e);
      } else
        broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), broadcast({
          type: "ERROR",
          code: StateStreamErrorCode.RECONNECT_FAILED,
          message: \`Maximum reconnection attempts (\${maxReconnectAttempts}) reached. Connection lost.\`
        });
      return;
    }
    broadcast({ type: "DISCONNECTED" }), broadcast({
      type: "ERROR",
      code: StateStreamErrorCode.CONNECTION_LOST,
      message: \`Stream closed with unexpected code: \${n.code}. Stopping stream.\`
    });
  };
}
function handleCommand(a, f) {
  switch (a.type) {
    case "START":
      maxAuthAttempts = a.maxAuthAttempts ?? DEFAULT_MAX_AUTH_ATTEMPTS, maxReconnectAttempts = a.maxReconnectAttempts ?? DEFAULT_MAX_RECONNECT_ATTEMPTS, reconnectDelay = a.reconnectDelay ?? DEFAULT_DELAY_RECONNECT, activePorts.add(f), socket && socket.readyState === WebSocket.OPEN && currentAddr === a.addr ? (f.postMessage({ type: "CONNECTED" }), f.postMessage({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED })) : connect(a.addr, a.token, a.isBinary, a.mode);
      break;
    case "STOP":
      activePorts.delete(f);
      for (const [e, r] of subscriptions.entries())
        r.delete(f), r.size === 0 && (subscriptions.delete(e), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [e] })));
      f.postMessage({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), activePorts.size === 0 && stopAndCleanup();
      break;
    case "UPDATE_TOKEN":
      const h = currentToken;
      if (currentToken = a.token, currentMode === "remote") {
        const e = socket && socket.readyState === WebSocket.OPEN;
        if (e && h === a.token)
          return;
        e ? sendAuth() : h !== a.token && currentAddr && activePorts.size > 0 && connect(currentAddr, currentToken, isBinaryMode, currentMode);
      }
      break;
    case "SUBSCRIBE":
      let c = subscriptions.get(a.guid);
      c || (c = /* @__PURE__ */ new Set(), subscriptions.set(a.guid, c));
      const d = c.size === 0;
      c.add(f), d && (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ subscribe: [a.guid] }));
      break;
    case "UNSUBSCRIBE":
      const n = subscriptions.get(a.guid);
      n && (n.delete(f), n.size === 0 && (subscriptions.delete(a.guid), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [a.guid] }))));
      break;
  }
}
if ("SharedWorkerGlobalScope" in self) {
  const a = self;
  a.onconnect = (f) => {
    const h = f.ports[0];
    h && (h.onmessage = (c) => handleCommand(c.data, h), h.start());
  };
} else {
  const a = self;
  a.onmessage = (f) => {
    handleCommand(f.data, a);
  };
}
`,Ce=typeof self<"u"&&self.Blob&&new Blob(["URL.revokeObjectURL(import.meta.url);",Ke],{type:"text/javascript;charset=utf-8"});function Vr(t){let e;try{if(e=Ce&&(self.URL||self.webkitURL).createObjectURL(Ce),!e)throw"";const r=new Worker(e,{type:"module",name:t?.name});return r.addEventListener("error",()=>{(self.URL||self.webkitURL).revokeObjectURL(e)}),r}catch{return new Worker("data:text/javascript;charset=utf-8,"+encodeURIComponent(Ke),{type:"module",name:t?.name})}}const zr=`var commonjsGlobal = typeof globalThis < "u" ? globalThis : typeof window < "u" ? window : typeof global < "u" ? global : typeof self < "u" ? self : {}, src = { exports: {} }, indexLight = { exports: {} }, indexMinimal = {}, minimal = {}, aspromise, hasRequiredAspromise;
function requireAspromise() {
  if (hasRequiredAspromise) return aspromise;
  hasRequiredAspromise = 1, aspromise = a;
  function a(f, h) {
    for (var c = new Array(arguments.length - 1), d = 0, n = 2, e = !0; n < arguments.length; )
      c[d++] = arguments[n++];
    return new Promise(function(i, t) {
      c[d] = function(s) {
        if (e)
          if (e = !1, s)
            t(s);
          else {
            for (var u = new Array(arguments.length - 1), o = 0; o < u.length; )
              u[o++] = arguments[o];
            i.apply(null, u);
          }
      };
      try {
        f.apply(h || null, c);
      } catch (l) {
        e && (e = !1, t(l));
      }
    });
  }
  return aspromise;
}
var base64 = {}, hasRequiredBase64;
function requireBase64() {
  return hasRequiredBase64 || (hasRequiredBase64 = 1, function(a) {
    var f = a;
    f.length = function(r) {
      var i = r.length;
      if (!i)
        return 0;
      for (var t = 0; --i % 4 > 1 && r.charAt(i) === "="; )
        ++t;
      return Math.ceil(r.length * 3) / 4 - t;
    };
    for (var h = new Array(64), c = new Array(123), d = 0; d < 64; )
      c[h[d] = d < 26 ? d + 65 : d < 52 ? d + 71 : d < 62 ? d - 4 : d - 59 | 43] = d++;
    f.encode = function(r, i, t) {
      for (var l = null, s = [], u = 0, o = 0, p; i < t; ) {
        var y = r[i++];
        switch (o) {
          case 0:
            s[u++] = h[y >> 2], p = (y & 3) << 4, o = 1;
            break;
          case 1:
            s[u++] = h[p | y >> 4], p = (y & 15) << 2, o = 2;
            break;
          case 2:
            s[u++] = h[p | y >> 6], s[u++] = h[y & 63], o = 0;
            break;
        }
        u > 8191 && ((l || (l = [])).push(String.fromCharCode.apply(String, s)), u = 0);
      }
      return o && (s[u++] = h[p], s[u++] = 61, o === 1 && (s[u++] = 61)), l ? (u && l.push(String.fromCharCode.apply(String, s.slice(0, u))), l.join("")) : String.fromCharCode.apply(String, s.slice(0, u));
    };
    var n = "invalid encoding";
    f.decode = function(r, i, t) {
      for (var l = t, s = 0, u, o = 0; o < r.length; ) {
        var p = r.charCodeAt(o++);
        if (p === 61 && s > 1)
          break;
        if ((p = c[p]) === void 0)
          throw Error(n);
        switch (s) {
          case 0:
            u = p, s = 1;
            break;
          case 1:
            i[t++] = u << 2 | (p & 48) >> 4, u = p, s = 2;
            break;
          case 2:
            i[t++] = (u & 15) << 4 | (p & 60) >> 2, u = p, s = 3;
            break;
          case 3:
            i[t++] = (u & 3) << 6 | p, s = 0;
            break;
        }
      }
      if (s === 1)
        throw Error(n);
      return t - l;
    }, f.test = function(r) {
      return /^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$/.test(r);
    };
  }(base64)), base64;
}
var eventemitter, hasRequiredEventemitter;
function requireEventemitter() {
  if (hasRequiredEventemitter) return eventemitter;
  hasRequiredEventemitter = 1, eventemitter = a;
  function a() {
    this._listeners = {};
  }
  return a.prototype.on = function(h, c, d) {
    return (this._listeners[h] || (this._listeners[h] = [])).push({
      fn: c,
      ctx: d || this
    }), this;
  }, a.prototype.off = function(h, c) {
    if (h === void 0)
      this._listeners = {};
    else if (c === void 0)
      this._listeners[h] = [];
    else
      for (var d = this._listeners[h], n = 0; n < d.length; )
        d[n].fn === c ? d.splice(n, 1) : ++n;
    return this;
  }, a.prototype.emit = function(h) {
    var c = this._listeners[h];
    if (c) {
      for (var d = [], n = 1; n < arguments.length; )
        d.push(arguments[n++]);
      for (n = 0; n < c.length; )
        c[n].fn.apply(c[n++].ctx, d);
    }
    return this;
  }, eventemitter;
}
var float, hasRequiredFloat;
function requireFloat() {
  if (hasRequiredFloat) return float;
  hasRequiredFloat = 1, float = a(a);
  function a(n) {
    return typeof Float32Array < "u" ? function() {
      var e = new Float32Array([-0]), r = new Uint8Array(e.buffer), i = r[3] === 128;
      function t(o, p, y) {
        e[0] = o, p[y] = r[0], p[y + 1] = r[1], p[y + 2] = r[2], p[y + 3] = r[3];
      }
      function l(o, p, y) {
        e[0] = o, p[y] = r[3], p[y + 1] = r[2], p[y + 2] = r[1], p[y + 3] = r[0];
      }
      n.writeFloatLE = i ? t : l, n.writeFloatBE = i ? l : t;
      function s(o, p) {
        return r[0] = o[p], r[1] = o[p + 1], r[2] = o[p + 2], r[3] = o[p + 3], e[0];
      }
      function u(o, p) {
        return r[3] = o[p], r[2] = o[p + 1], r[1] = o[p + 2], r[0] = o[p + 3], e[0];
      }
      n.readFloatLE = i ? s : u, n.readFloatBE = i ? u : s;
    }() : function() {
      function e(i, t, l, s) {
        var u = t < 0 ? 1 : 0;
        if (u && (t = -t), t === 0)
          i(1 / t > 0 ? (
            /* positive */
            0
          ) : (
            /* negative 0 */
            2147483648
          ), l, s);
        else if (isNaN(t))
          i(2143289344, l, s);
        else if (t > 34028234663852886e22)
          i((u << 31 | 2139095040) >>> 0, l, s);
        else if (t < 11754943508222875e-54)
          i((u << 31 | Math.round(t / 1401298464324817e-60)) >>> 0, l, s);
        else {
          var o = Math.floor(Math.log(t) / Math.LN2), p = Math.round(t * Math.pow(2, -o) * 8388608) & 8388607;
          i((u << 31 | o + 127 << 23 | p) >>> 0, l, s);
        }
      }
      n.writeFloatLE = e.bind(null, f), n.writeFloatBE = e.bind(null, h);
      function r(i, t, l) {
        var s = i(t, l), u = (s >> 31) * 2 + 1, o = s >>> 23 & 255, p = s & 8388607;
        return o === 255 ? p ? NaN : u * (1 / 0) : o === 0 ? u * 1401298464324817e-60 * p : u * Math.pow(2, o - 150) * (p + 8388608);
      }
      n.readFloatLE = r.bind(null, c), n.readFloatBE = r.bind(null, d);
    }(), typeof Float64Array < "u" ? function() {
      var e = new Float64Array([-0]), r = new Uint8Array(e.buffer), i = r[7] === 128;
      function t(o, p, y) {
        e[0] = o, p[y] = r[0], p[y + 1] = r[1], p[y + 2] = r[2], p[y + 3] = r[3], p[y + 4] = r[4], p[y + 5] = r[5], p[y + 6] = r[6], p[y + 7] = r[7];
      }
      function l(o, p, y) {
        e[0] = o, p[y] = r[7], p[y + 1] = r[6], p[y + 2] = r[5], p[y + 3] = r[4], p[y + 4] = r[3], p[y + 5] = r[2], p[y + 6] = r[1], p[y + 7] = r[0];
      }
      n.writeDoubleLE = i ? t : l, n.writeDoubleBE = i ? l : t;
      function s(o, p) {
        return r[0] = o[p], r[1] = o[p + 1], r[2] = o[p + 2], r[3] = o[p + 3], r[4] = o[p + 4], r[5] = o[p + 5], r[6] = o[p + 6], r[7] = o[p + 7], e[0];
      }
      function u(o, p) {
        return r[7] = o[p], r[6] = o[p + 1], r[5] = o[p + 2], r[4] = o[p + 3], r[3] = o[p + 4], r[2] = o[p + 5], r[1] = o[p + 6], r[0] = o[p + 7], e[0];
      }
      n.readDoubleLE = i ? s : u, n.readDoubleBE = i ? u : s;
    }() : function() {
      function e(i, t, l, s, u, o) {
        var p = s < 0 ? 1 : 0;
        if (p && (s = -s), s === 0)
          i(0, u, o + t), i(1 / s > 0 ? (
            /* positive */
            0
          ) : (
            /* negative 0 */
            2147483648
          ), u, o + l);
        else if (isNaN(s))
          i(0, u, o + t), i(2146959360, u, o + l);
        else if (s > 17976931348623157e292)
          i(0, u, o + t), i((p << 31 | 2146435072) >>> 0, u, o + l);
        else {
          var y;
          if (s < 22250738585072014e-324)
            y = s / 5e-324, i(y >>> 0, u, o + t), i((p << 31 | y / 4294967296) >>> 0, u, o + l);
          else {
            var E = Math.floor(Math.log(s) / Math.LN2);
            E === 1024 && (E = 1023), y = s * Math.pow(2, -E), i(y * 4503599627370496 >>> 0, u, o + t), i((p << 31 | E + 1023 << 20 | y * 1048576 & 1048575) >>> 0, u, o + l);
          }
        }
      }
      n.writeDoubleLE = e.bind(null, f, 0, 4), n.writeDoubleBE = e.bind(null, h, 4, 0);
      function r(i, t, l, s, u) {
        var o = i(s, u + t), p = i(s, u + l), y = (p >> 31) * 2 + 1, E = p >>> 20 & 2047, v = 4294967296 * (p & 1048575) + o;
        return E === 2047 ? v ? NaN : y * (1 / 0) : E === 0 ? y * 5e-324 * v : y * Math.pow(2, E - 1075) * (v + 4503599627370496);
      }
      n.readDoubleLE = r.bind(null, c, 0, 4), n.readDoubleBE = r.bind(null, d, 4, 0);
    }(), n;
  }
  function f(n, e, r) {
    e[r] = n & 255, e[r + 1] = n >>> 8 & 255, e[r + 2] = n >>> 16 & 255, e[r + 3] = n >>> 24;
  }
  function h(n, e, r) {
    e[r] = n >>> 24, e[r + 1] = n >>> 16 & 255, e[r + 2] = n >>> 8 & 255, e[r + 3] = n & 255;
  }
  function c(n, e) {
    return (n[e] | n[e + 1] << 8 | n[e + 2] << 16 | n[e + 3] << 24) >>> 0;
  }
  function d(n, e) {
    return (n[e] << 24 | n[e + 1] << 16 | n[e + 2] << 8 | n[e + 3]) >>> 0;
  }
  return float;
}
var inquire_1, hasRequiredInquire;
function requireInquire() {
  if (hasRequiredInquire) return inquire_1;
  hasRequiredInquire = 1, inquire_1 = inquire;
  function inquire(moduleName) {
    try {
      var mod = eval("quire".replace(/^/, "re"))(moduleName);
      if (mod && (mod.length || Object.keys(mod).length))
        return mod;
    } catch (a) {
    }
    return null;
  }
  return inquire_1;
}
var utf8 = {}, hasRequiredUtf8;
function requireUtf8() {
  return hasRequiredUtf8 || (hasRequiredUtf8 = 1, function(a) {
    var f = a;
    f.length = function(c) {
      for (var d = 0, n = 0, e = 0; e < c.length; ++e)
        n = c.charCodeAt(e), n < 128 ? d += 1 : n < 2048 ? d += 2 : (n & 64512) === 55296 && (c.charCodeAt(e + 1) & 64512) === 56320 ? (++e, d += 4) : d += 3;
      return d;
    }, f.read = function(c, d, n) {
      var e = n - d;
      if (e < 1)
        return "";
      for (var r = null, i = [], t = 0, l; d < n; )
        l = c[d++], l < 128 ? i[t++] = l : l > 191 && l < 224 ? i[t++] = (l & 31) << 6 | c[d++] & 63 : l > 239 && l < 365 ? (l = ((l & 7) << 18 | (c[d++] & 63) << 12 | (c[d++] & 63) << 6 | c[d++] & 63) - 65536, i[t++] = 55296 + (l >> 10), i[t++] = 56320 + (l & 1023)) : i[t++] = (l & 15) << 12 | (c[d++] & 63) << 6 | c[d++] & 63, t > 8191 && ((r || (r = [])).push(String.fromCharCode.apply(String, i)), t = 0);
      return r ? (t && r.push(String.fromCharCode.apply(String, i.slice(0, t))), r.join("")) : String.fromCharCode.apply(String, i.slice(0, t));
    }, f.write = function(c, d, n) {
      for (var e = n, r, i, t = 0; t < c.length; ++t)
        r = c.charCodeAt(t), r < 128 ? d[n++] = r : r < 2048 ? (d[n++] = r >> 6 | 192, d[n++] = r & 63 | 128) : (r & 64512) === 55296 && ((i = c.charCodeAt(t + 1)) & 64512) === 56320 ? (r = 65536 + ((r & 1023) << 10) + (i & 1023), ++t, d[n++] = r >> 18 | 240, d[n++] = r >> 12 & 63 | 128, d[n++] = r >> 6 & 63 | 128, d[n++] = r & 63 | 128) : (d[n++] = r >> 12 | 224, d[n++] = r >> 6 & 63 | 128, d[n++] = r & 63 | 128);
      return n - e;
    };
  }(utf8)), utf8;
}
var pool_1, hasRequiredPool;
function requirePool() {
  if (hasRequiredPool) return pool_1;
  hasRequiredPool = 1, pool_1 = a;
  function a(f, h, c) {
    var d = c || 8192, n = d >>> 1, e = null, r = d;
    return function(t) {
      if (t < 1 || t > n)
        return f(t);
      r + t > d && (e = f(d), r = 0);
      var l = h.call(e, r, r += t);
      return r & 7 && (r = (r | 7) + 1), l;
    };
  }
  return pool_1;
}
var longbits, hasRequiredLongbits;
function requireLongbits() {
  if (hasRequiredLongbits) return longbits;
  hasRequiredLongbits = 1, longbits = f;
  var a = requireMinimal();
  function f(n, e) {
    this.lo = n >>> 0, this.hi = e >>> 0;
  }
  var h = f.zero = new f(0, 0);
  h.toNumber = function() {
    return 0;
  }, h.zzEncode = h.zzDecode = function() {
    return this;
  }, h.length = function() {
    return 1;
  };
  var c = f.zeroHash = "\\0\\0\\0\\0\\0\\0\\0\\0";
  f.fromNumber = function(e) {
    if (e === 0)
      return h;
    var r = e < 0;
    r && (e = -e);
    var i = e >>> 0, t = (e - i) / 4294967296 >>> 0;
    return r && (t = ~t >>> 0, i = ~i >>> 0, ++i > 4294967295 && (i = 0, ++t > 4294967295 && (t = 0))), new f(i, t);
  }, f.from = function(e) {
    if (typeof e == "number")
      return f.fromNumber(e);
    if (a.isString(e))
      if (a.Long)
        e = a.Long.fromString(e);
      else
        return f.fromNumber(parseInt(e, 10));
    return e.low || e.high ? new f(e.low >>> 0, e.high >>> 0) : h;
  }, f.prototype.toNumber = function(e) {
    if (!e && this.hi >>> 31) {
      var r = ~this.lo + 1 >>> 0, i = ~this.hi >>> 0;
      return r || (i = i + 1 >>> 0), -(r + i * 4294967296);
    }
    return this.lo + this.hi * 4294967296;
  }, f.prototype.toLong = function(e) {
    return a.Long ? new a.Long(this.lo | 0, this.hi | 0, !!e) : { low: this.lo | 0, high: this.hi | 0, unsigned: !!e };
  };
  var d = String.prototype.charCodeAt;
  return f.fromHash = function(e) {
    return e === c ? h : new f(
      (d.call(e, 0) | d.call(e, 1) << 8 | d.call(e, 2) << 16 | d.call(e, 3) << 24) >>> 0,
      (d.call(e, 4) | d.call(e, 5) << 8 | d.call(e, 6) << 16 | d.call(e, 7) << 24) >>> 0
    );
  }, f.prototype.toHash = function() {
    return String.fromCharCode(
      this.lo & 255,
      this.lo >>> 8 & 255,
      this.lo >>> 16 & 255,
      this.lo >>> 24,
      this.hi & 255,
      this.hi >>> 8 & 255,
      this.hi >>> 16 & 255,
      this.hi >>> 24
    );
  }, f.prototype.zzEncode = function() {
    var e = this.hi >> 31;
    return this.hi = ((this.hi << 1 | this.lo >>> 31) ^ e) >>> 0, this.lo = (this.lo << 1 ^ e) >>> 0, this;
  }, f.prototype.zzDecode = function() {
    var e = -(this.lo & 1);
    return this.lo = ((this.lo >>> 1 | this.hi << 31) ^ e) >>> 0, this.hi = (this.hi >>> 1 ^ e) >>> 0, this;
  }, f.prototype.length = function() {
    var e = this.lo, r = (this.lo >>> 28 | this.hi << 4) >>> 0, i = this.hi >>> 24;
    return i === 0 ? r === 0 ? e < 16384 ? e < 128 ? 1 : 2 : e < 2097152 ? 3 : 4 : r < 16384 ? r < 128 ? 5 : 6 : r < 2097152 ? 7 : 8 : i < 128 ? 9 : 10;
  }, longbits;
}
var hasRequiredMinimal;
function requireMinimal() {
  return hasRequiredMinimal || (hasRequiredMinimal = 1, function(a) {
    var f = a;
    f.asPromise = requireAspromise(), f.base64 = requireBase64(), f.EventEmitter = requireEventemitter(), f.float = requireFloat(), f.inquire = requireInquire(), f.utf8 = requireUtf8(), f.pool = requirePool(), f.LongBits = requireLongbits(), f.isNode = !!(typeof commonjsGlobal < "u" && commonjsGlobal && commonjsGlobal.process && commonjsGlobal.process.versions && commonjsGlobal.process.versions.node), f.global = f.isNode && commonjsGlobal || typeof window < "u" && window || typeof self < "u" && self || minimal, f.emptyArray = Object.freeze ? Object.freeze([]) : (
      /* istanbul ignore next */
      []
    ), f.emptyObject = Object.freeze ? Object.freeze({}) : (
      /* istanbul ignore next */
      {}
    ), f.isInteger = Number.isInteger || /* istanbul ignore next */
    function(n) {
      return typeof n == "number" && isFinite(n) && Math.floor(n) === n;
    }, f.isString = function(n) {
      return typeof n == "string" || n instanceof String;
    }, f.isObject = function(n) {
      return n && typeof n == "object";
    }, f.isset = /**
     * Checks if a property on a message is considered to be present.
     * @param {Object} obj Plain object or message instance
     * @param {string} prop Property name
     * @returns {boolean} \`true\` if considered to be present, otherwise \`false\`
     */
    f.isSet = function(n, e) {
      var r = n[e];
      return r != null && n.hasOwnProperty(e) ? typeof r != "object" || (Array.isArray(r) ? r.length : Object.keys(r).length) > 0 : !1;
    }, f.Buffer = function() {
      try {
        var d = f.inquire("buffer").Buffer;
        return d.prototype.utf8Write ? d : (
          /* istanbul ignore next */
          null
        );
      } catch {
        return null;
      }
    }(), f._Buffer_from = null, f._Buffer_allocUnsafe = null, f.newBuffer = function(n) {
      return typeof n == "number" ? f.Buffer ? f._Buffer_allocUnsafe(n) : new f.Array(n) : f.Buffer ? f._Buffer_from(n) : typeof Uint8Array > "u" ? n : new Uint8Array(n);
    }, f.Array = typeof Uint8Array < "u" ? Uint8Array : Array, f.Long = /* istanbul ignore next */
    f.global.dcodeIO && /* istanbul ignore next */
    f.global.dcodeIO.Long || /* istanbul ignore next */
    f.global.Long || f.inquire("long"), f.key2Re = /^true|false|0|1$/, f.key32Re = /^-?(?:0|[1-9][0-9]*)$/, f.key64Re = /^(?:[\\\\x00-\\\\xff]{8}|-?(?:0|[1-9][0-9]*))$/, f.longToHash = function(n) {
      return n ? f.LongBits.from(n).toHash() : f.LongBits.zeroHash;
    }, f.longFromHash = function(n, e) {
      var r = f.LongBits.fromHash(n);
      return f.Long ? f.Long.fromBits(r.lo, r.hi, e) : r.toNumber(!!e);
    };
    function h(d, n, e) {
      for (var r = Object.keys(n), i = 0; i < r.length; ++i)
        (d[r[i]] === void 0 || !e) && (d[r[i]] = n[r[i]]);
      return d;
    }
    f.merge = h, f.lcFirst = function(n) {
      return n.charAt(0).toLowerCase() + n.substring(1);
    };
    function c(d) {
      function n(e, r) {
        if (!(this instanceof n))
          return new n(e, r);
        Object.defineProperty(this, "message", { get: function() {
          return e;
        } }), Error.captureStackTrace ? Error.captureStackTrace(this, n) : Object.defineProperty(this, "stack", { value: new Error().stack || "" }), r && h(this, r);
      }
      return n.prototype = Object.create(Error.prototype, {
        constructor: {
          value: n,
          writable: !0,
          enumerable: !1,
          configurable: !0
        },
        name: {
          get: function() {
            return d;
          },
          set: void 0,
          enumerable: !1,
          // configurable: false would accurately preserve the behavior of
          // the original, but I'm guessing that was not intentional.
          // For an actual error subclass, this property would
          // be configurable.
          configurable: !0
        },
        toString: {
          value: function() {
            return this.name + ": " + this.message;
          },
          writable: !0,
          enumerable: !1,
          configurable: !0
        }
      }), n;
    }
    f.newError = c, f.ProtocolError = c("ProtocolError"), f.oneOfGetter = function(n) {
      for (var e = {}, r = 0; r < n.length; ++r)
        e[n[r]] = 1;
      return function() {
        for (var i = Object.keys(this), t = i.length - 1; t > -1; --t)
          if (e[i[t]] === 1 && this[i[t]] !== void 0 && this[i[t]] !== null)
            return i[t];
      };
    }, f.oneOfSetter = function(n) {
      return function(e) {
        for (var r = 0; r < n.length; ++r)
          n[r] !== e && delete this[n[r]];
      };
    }, f.toJSONOptions = {
      longs: String,
      enums: String,
      bytes: String,
      json: !0
    }, f._configure = function() {
      var d = f.Buffer;
      if (!d) {
        f._Buffer_from = f._Buffer_allocUnsafe = null;
        return;
      }
      f._Buffer_from = d.from !== Uint8Array.from && d.from || /* istanbul ignore next */
      function(e, r) {
        return new d(e, r);
      }, f._Buffer_allocUnsafe = d.allocUnsafe || /* istanbul ignore next */
      function(e) {
        return new d(e);
      };
    };
  }(minimal)), minimal;
}
var writer, hasRequiredWriter;
function requireWriter() {
  if (hasRequiredWriter) return writer;
  hasRequiredWriter = 1, writer = i;
  var a = requireMinimal(), f, h = a.LongBits, c = a.base64, d = a.utf8;
  function n(E, v, m) {
    this.fn = E, this.len = v, this.next = void 0, this.val = m;
  }
  function e() {
  }
  function r(E) {
    this.head = E.head, this.tail = E.tail, this.len = E.len, this.next = E.states;
  }
  function i() {
    this.len = 0, this.head = new n(e, 0, 0), this.tail = this.head, this.states = null;
  }
  var t = function() {
    return a.Buffer ? function() {
      return (i.create = function() {
        return new f();
      })();
    } : function() {
      return new i();
    };
  };
  i.create = t(), i.alloc = function(v) {
    return new a.Array(v);
  }, a.Array !== Array && (i.alloc = a.pool(i.alloc, a.Array.prototype.subarray)), i.prototype._push = function(v, m, _) {
    return this.tail = this.tail.next = new n(v, m, _), this.len += m, this;
  };
  function l(E, v, m) {
    v[m] = E & 255;
  }
  function s(E, v, m) {
    for (; E > 127; )
      v[m++] = E & 127 | 128, E >>>= 7;
    v[m] = E;
  }
  function u(E, v) {
    this.len = E, this.next = void 0, this.val = v;
  }
  u.prototype = Object.create(n.prototype), u.prototype.fn = s, i.prototype.uint32 = function(v) {
    return this.len += (this.tail = this.tail.next = new u(
      (v = v >>> 0) < 128 ? 1 : v < 16384 ? 2 : v < 2097152 ? 3 : v < 268435456 ? 4 : 5,
      v
    )).len, this;
  }, i.prototype.int32 = function(v) {
    return v < 0 ? this._push(o, 10, h.fromNumber(v)) : this.uint32(v);
  }, i.prototype.sint32 = function(v) {
    return this.uint32((v << 1 ^ v >> 31) >>> 0);
  };
  function o(E, v, m) {
    for (; E.hi; )
      v[m++] = E.lo & 127 | 128, E.lo = (E.lo >>> 7 | E.hi << 25) >>> 0, E.hi >>>= 7;
    for (; E.lo > 127; )
      v[m++] = E.lo & 127 | 128, E.lo = E.lo >>> 7;
    v[m++] = E.lo;
  }
  i.prototype.uint64 = function(v) {
    var m = h.from(v);
    return this._push(o, m.length(), m);
  }, i.prototype.int64 = i.prototype.uint64, i.prototype.sint64 = function(v) {
    var m = h.from(v).zzEncode();
    return this._push(o, m.length(), m);
  }, i.prototype.bool = function(v) {
    return this._push(l, 1, v ? 1 : 0);
  };
  function p(E, v, m) {
    v[m] = E & 255, v[m + 1] = E >>> 8 & 255, v[m + 2] = E >>> 16 & 255, v[m + 3] = E >>> 24;
  }
  i.prototype.fixed32 = function(v) {
    return this._push(p, 4, v >>> 0);
  }, i.prototype.sfixed32 = i.prototype.fixed32, i.prototype.fixed64 = function(v) {
    var m = h.from(v);
    return this._push(p, 4, m.lo)._push(p, 4, m.hi);
  }, i.prototype.sfixed64 = i.prototype.fixed64, i.prototype.float = function(v) {
    return this._push(a.float.writeFloatLE, 4, v);
  }, i.prototype.double = function(v) {
    return this._push(a.float.writeDoubleLE, 8, v);
  };
  var y = a.Array.prototype.set ? function(v, m, _) {
    m.set(v, _);
  } : function(v, m, _) {
    for (var A = 0; A < v.length; ++A)
      m[_ + A] = v[A];
  };
  return i.prototype.bytes = function(v) {
    var m = v.length >>> 0;
    if (!m)
      return this._push(l, 1, 0);
    if (a.isString(v)) {
      var _ = i.alloc(m = c.length(v));
      c.decode(v, _, 0), v = _;
    }
    return this.uint32(m)._push(y, m, v);
  }, i.prototype.string = function(v) {
    var m = d.length(v);
    return m ? this.uint32(m)._push(d.write, m, v) : this._push(l, 1, 0);
  }, i.prototype.fork = function() {
    return this.states = new r(this), this.head = this.tail = new n(e, 0, 0), this.len = 0, this;
  }, i.prototype.reset = function() {
    return this.states ? (this.head = this.states.head, this.tail = this.states.tail, this.len = this.states.len, this.states = this.states.next) : (this.head = this.tail = new n(e, 0, 0), this.len = 0), this;
  }, i.prototype.ldelim = function() {
    var v = this.head, m = this.tail, _ = this.len;
    return this.reset().uint32(_), _ && (this.tail.next = v.next, this.tail = m, this.len += _), this;
  }, i.prototype.finish = function() {
    for (var v = this.head.next, m = this.constructor.alloc(this.len), _ = 0; v; )
      v.fn(v.val, m, _), _ += v.len, v = v.next;
    return m;
  }, i._configure = function(E) {
    f = E, i.create = t(), f._configure();
  }, writer;
}
var writer_buffer, hasRequiredWriter_buffer;
function requireWriter_buffer() {
  if (hasRequiredWriter_buffer) return writer_buffer;
  hasRequiredWriter_buffer = 1, writer_buffer = h;
  var a = requireWriter();
  (h.prototype = Object.create(a.prototype)).constructor = h;
  var f = requireMinimal();
  function h() {
    a.call(this);
  }
  h._configure = function() {
    h.alloc = f._Buffer_allocUnsafe, h.writeBytesBuffer = f.Buffer && f.Buffer.prototype instanceof Uint8Array && f.Buffer.prototype.set.name === "set" ? function(n, e, r) {
      e.set(n, r);
    } : function(n, e, r) {
      if (n.copy)
        n.copy(e, r, 0, n.length);
      else for (var i = 0; i < n.length; )
        e[r++] = n[i++];
    };
  }, h.prototype.bytes = function(n) {
    f.isString(n) && (n = f._Buffer_from(n, "base64"));
    var e = n.length >>> 0;
    return this.uint32(e), e && this._push(h.writeBytesBuffer, e, n), this;
  };
  function c(d, n, e) {
    d.length < 40 ? f.utf8.write(d, n, e) : n.utf8Write ? n.utf8Write(d, e) : n.write(d, e);
  }
  return h.prototype.string = function(n) {
    var e = f.Buffer.byteLength(n);
    return this.uint32(e), e && this._push(c, e, n), this;
  }, h._configure(), writer_buffer;
}
var reader, hasRequiredReader;
function requireReader() {
  if (hasRequiredReader) return reader;
  hasRequiredReader = 1, reader = n;
  var a = requireMinimal(), f, h = a.LongBits, c = a.utf8;
  function d(s, u) {
    return RangeError("index out of range: " + s.pos + " + " + (u || 1) + " > " + s.len);
  }
  function n(s) {
    this.buf = s, this.pos = 0, this.len = s.length;
  }
  var e = typeof Uint8Array < "u" ? function(u) {
    if (u instanceof Uint8Array || Array.isArray(u))
      return new n(u);
    throw Error("illegal buffer");
  } : function(u) {
    if (Array.isArray(u))
      return new n(u);
    throw Error("illegal buffer");
  }, r = function() {
    return a.Buffer ? function(o) {
      return (n.create = function(y) {
        return a.Buffer.isBuffer(y) ? new f(y) : e(y);
      })(o);
    } : e;
  };
  n.create = r(), n.prototype._slice = a.Array.prototype.subarray || /* istanbul ignore next */
  a.Array.prototype.slice, n.prototype.uint32 = /* @__PURE__ */ function() {
    var u = 4294967295;
    return function() {
      if (u = (this.buf[this.pos] & 127) >>> 0, this.buf[this.pos++] < 128 || (u = (u | (this.buf[this.pos] & 127) << 7) >>> 0, this.buf[this.pos++] < 128) || (u = (u | (this.buf[this.pos] & 127) << 14) >>> 0, this.buf[this.pos++] < 128) || (u = (u | (this.buf[this.pos] & 127) << 21) >>> 0, this.buf[this.pos++] < 128) || (u = (u | (this.buf[this.pos] & 15) << 28) >>> 0, this.buf[this.pos++] < 128)) return u;
      if ((this.pos += 5) > this.len)
        throw this.pos = this.len, d(this, 10);
      return u;
    };
  }(), n.prototype.int32 = function() {
    return this.uint32() | 0;
  }, n.prototype.sint32 = function() {
    var u = this.uint32();
    return u >>> 1 ^ -(u & 1) | 0;
  };
  function i() {
    var s = new h(0, 0), u = 0;
    if (this.len - this.pos > 4) {
      for (; u < 4; ++u)
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << u * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      if (s.lo = (s.lo | (this.buf[this.pos] & 127) << 28) >>> 0, s.hi = (s.hi | (this.buf[this.pos] & 127) >> 4) >>> 0, this.buf[this.pos++] < 128)
        return s;
      u = 0;
    } else {
      for (; u < 3; ++u) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << u * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
      return s.lo = (s.lo | (this.buf[this.pos++] & 127) << u * 7) >>> 0, s;
    }
    if (this.len - this.pos > 4) {
      for (; u < 5; ++u)
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << u * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
    } else
      for (; u < 5; ++u) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << u * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
    throw Error("invalid varint encoding");
  }
  n.prototype.bool = function() {
    return this.uint32() !== 0;
  };
  function t(s, u) {
    return (s[u - 4] | s[u - 3] << 8 | s[u - 2] << 16 | s[u - 1] << 24) >>> 0;
  }
  n.prototype.fixed32 = function() {
    if (this.pos + 4 > this.len)
      throw d(this, 4);
    return t(this.buf, this.pos += 4);
  }, n.prototype.sfixed32 = function() {
    if (this.pos + 4 > this.len)
      throw d(this, 4);
    return t(this.buf, this.pos += 4) | 0;
  };
  function l() {
    if (this.pos + 8 > this.len)
      throw d(this, 8);
    return new h(t(this.buf, this.pos += 4), t(this.buf, this.pos += 4));
  }
  return n.prototype.float = function() {
    if (this.pos + 4 > this.len)
      throw d(this, 4);
    var u = a.float.readFloatLE(this.buf, this.pos);
    return this.pos += 4, u;
  }, n.prototype.double = function() {
    if (this.pos + 8 > this.len)
      throw d(this, 4);
    var u = a.float.readDoubleLE(this.buf, this.pos);
    return this.pos += 8, u;
  }, n.prototype.bytes = function() {
    var u = this.uint32(), o = this.pos, p = this.pos + u;
    if (p > this.len)
      throw d(this, u);
    if (this.pos += u, Array.isArray(this.buf))
      return this.buf.slice(o, p);
    if (o === p) {
      var y = a.Buffer;
      return y ? y.alloc(0) : new this.buf.constructor(0);
    }
    return this._slice.call(this.buf, o, p);
  }, n.prototype.string = function() {
    var u = this.bytes();
    return c.read(u, 0, u.length);
  }, n.prototype.skip = function(u) {
    if (typeof u == "number") {
      if (this.pos + u > this.len)
        throw d(this, u);
      this.pos += u;
    } else
      do
        if (this.pos >= this.len)
          throw d(this);
      while (this.buf[this.pos++] & 128);
    return this;
  }, n.prototype.skipType = function(s) {
    switch (s) {
      case 0:
        this.skip();
        break;
      case 1:
        this.skip(8);
        break;
      case 2:
        this.skip(this.uint32());
        break;
      case 3:
        for (; (s = this.uint32() & 7) !== 4; )
          this.skipType(s);
        break;
      case 5:
        this.skip(4);
        break;
      /* istanbul ignore next */
      default:
        throw Error("invalid wire type " + s + " at offset " + this.pos);
    }
    return this;
  }, n._configure = function(s) {
    f = s, n.create = r(), f._configure();
    var u = a.Long ? "toLong" : (
      /* istanbul ignore next */
      "toNumber"
    );
    a.merge(n.prototype, {
      int64: function() {
        return i.call(this)[u](!1);
      },
      uint64: function() {
        return i.call(this)[u](!0);
      },
      sint64: function() {
        return i.call(this).zzDecode()[u](!1);
      },
      fixed64: function() {
        return l.call(this)[u](!0);
      },
      sfixed64: function() {
        return l.call(this)[u](!1);
      }
    });
  }, reader;
}
var reader_buffer, hasRequiredReader_buffer;
function requireReader_buffer() {
  if (hasRequiredReader_buffer) return reader_buffer;
  hasRequiredReader_buffer = 1, reader_buffer = h;
  var a = requireReader();
  (h.prototype = Object.create(a.prototype)).constructor = h;
  var f = requireMinimal();
  function h(c) {
    a.call(this, c);
  }
  return h._configure = function() {
    f.Buffer && (h.prototype._slice = f.Buffer.prototype.slice);
  }, h.prototype.string = function() {
    var d = this.uint32();
    return this.buf.utf8Slice ? this.buf.utf8Slice(this.pos, this.pos = Math.min(this.pos + d, this.len)) : this.buf.toString("utf-8", this.pos, this.pos = Math.min(this.pos + d, this.len));
  }, h._configure(), reader_buffer;
}
var rpc = {}, service$1, hasRequiredService$1;
function requireService$1() {
  if (hasRequiredService$1) return service$1;
  hasRequiredService$1 = 1, service$1 = f;
  var a = requireMinimal();
  (f.prototype = Object.create(a.EventEmitter.prototype)).constructor = f;
  function f(h, c, d) {
    if (typeof h != "function")
      throw TypeError("rpcImpl must be a function");
    a.EventEmitter.call(this), this.rpcImpl = h, this.requestDelimited = !!c, this.responseDelimited = !!d;
  }
  return f.prototype.rpcCall = function h(c, d, n, e, r) {
    if (!e)
      throw TypeError("request must be specified");
    var i = this;
    if (!r)
      return a.asPromise(h, i, c, d, n, e);
    if (!i.rpcImpl) {
      setTimeout(function() {
        r(Error("already ended"));
      }, 0);
      return;
    }
    try {
      return i.rpcImpl(
        c,
        d[i.requestDelimited ? "encodeDelimited" : "encode"](e).finish(),
        function(l, s) {
          if (l)
            return i.emit("error", l, c), r(l);
          if (s === null) {
            i.end(
              /* endedByRPC */
              !0
            );
            return;
          }
          if (!(s instanceof n))
            try {
              s = n[i.responseDelimited ? "decodeDelimited" : "decode"](s);
            } catch (u) {
              return i.emit("error", u, c), r(u);
            }
          return i.emit("data", s, c), r(null, s);
        }
      );
    } catch (t) {
      i.emit("error", t, c), setTimeout(function() {
        r(t);
      }, 0);
      return;
    }
  }, f.prototype.end = function(c) {
    return this.rpcImpl && (c || this.rpcImpl(null, null, null), this.rpcImpl = null, this.emit("end").off()), this;
  }, service$1;
}
var hasRequiredRpc;
function requireRpc() {
  return hasRequiredRpc || (hasRequiredRpc = 1, function(a) {
    var f = a;
    f.Service = requireService$1();
  }(rpc)), rpc;
}
var roots, hasRequiredRoots;
function requireRoots() {
  return hasRequiredRoots || (hasRequiredRoots = 1, roots = {}), roots;
}
var hasRequiredIndexMinimal;
function requireIndexMinimal() {
  return hasRequiredIndexMinimal || (hasRequiredIndexMinimal = 1, function(a) {
    var f = a;
    f.build = "minimal", f.Writer = requireWriter(), f.BufferWriter = requireWriter_buffer(), f.Reader = requireReader(), f.BufferReader = requireReader_buffer(), f.util = requireMinimal(), f.rpc = requireRpc(), f.roots = requireRoots(), f.configure = h;
    function h() {
      f.util._configure(), f.Writer._configure(f.BufferWriter), f.Reader._configure(f.BufferReader);
    }
    h();
  }(indexMinimal)), indexMinimal;
}
var types = {}, util = { exports: {} }, codegen_1, hasRequiredCodegen;
function requireCodegen() {
  if (hasRequiredCodegen) return codegen_1;
  hasRequiredCodegen = 1, codegen_1 = a;
  function a(f, h) {
    typeof f == "string" && (h = f, f = void 0);
    var c = [];
    function d(e) {
      if (typeof e != "string") {
        var r = n();
        if (a.verbose && console.log("codegen: " + r), r = "return " + r, e) {
          for (var i = Object.keys(e), t = new Array(i.length + 1), l = new Array(i.length), s = 0; s < i.length; )
            t[s] = i[s], l[s] = e[i[s++]];
          return t[s] = r, Function.apply(null, t).apply(null, l);
        }
        return Function(r)();
      }
      for (var u = new Array(arguments.length - 1), o = 0; o < u.length; )
        u[o] = arguments[++o];
      if (o = 0, e = e.replace(/%([%dfijs])/g, function(y, E) {
        var v = u[o++];
        switch (E) {
          case "d":
          case "f":
            return String(Number(v));
          case "i":
            return String(Math.floor(v));
          case "j":
            return JSON.stringify(v);
          case "s":
            return String(v);
        }
        return "%";
      }), o !== u.length)
        throw Error("parameter count mismatch");
      return c.push(e), d;
    }
    function n(e) {
      return "function " + (e || h || "") + "(" + (f && f.join(",") || "") + \`){
  \` + c.join(\`
  \`) + \`
}\`;
    }
    return d.toString = n, d;
  }
  return a.verbose = !1, codegen_1;
}
var fetch_1, hasRequiredFetch;
function requireFetch() {
  if (hasRequiredFetch) return fetch_1;
  hasRequiredFetch = 1, fetch_1 = c;
  var a = requireAspromise(), f = requireInquire(), h = f("fs");
  function c(d, n, e) {
    return typeof n == "function" ? (e = n, n = {}) : n || (n = {}), e ? !n.xhr && h && h.readFile ? h.readFile(d, function(i, t) {
      return i && typeof XMLHttpRequest < "u" ? c.xhr(d, n, e) : i ? e(i) : e(null, n.binary ? t : t.toString("utf8"));
    }) : c.xhr(d, n, e) : a(c, this, d, n);
  }
  return c.xhr = function(n, e, r) {
    var i = new XMLHttpRequest();
    i.onreadystatechange = function() {
      if (i.readyState === 4) {
        if (i.status !== 0 && i.status !== 200)
          return r(Error("status " + i.status));
        if (e.binary) {
          var l = i.response;
          if (!l) {
            l = [];
            for (var s = 0; s < i.responseText.length; ++s)
              l.push(i.responseText.charCodeAt(s) & 255);
          }
          return r(null, typeof Uint8Array < "u" ? new Uint8Array(l) : l);
        }
        return r(null, i.responseText);
      }
    }, e.binary && ("overrideMimeType" in i && i.overrideMimeType("text/plain; charset=x-user-defined"), i.responseType = "arraybuffer"), i.open("GET", n), i.send();
  }, fetch_1;
}
var path = {}, hasRequiredPath;
function requirePath() {
  return hasRequiredPath || (hasRequiredPath = 1, function(a) {
    var f = a, h = (
      /**
       * Tests if the specified path is absolute.
       * @param {string} path Path to test
       * @returns {boolean} \`true\` if path is absolute
       */
      f.isAbsolute = function(n) {
        return /^(?:\\/|\\w+:)/.test(n);
      }
    ), c = (
      /**
       * Normalizes the specified path.
       * @param {string} path Path to normalize
       * @returns {string} Normalized path
       */
      f.normalize = function(n) {
        n = n.replace(/\\\\/g, "/").replace(/\\/{2,}/g, "/");
        var e = n.split("/"), r = h(n), i = "";
        r && (i = e.shift() + "/");
        for (var t = 0; t < e.length; )
          e[t] === ".." ? t > 0 && e[t - 1] !== ".." ? e.splice(--t, 2) : r ? e.splice(t, 1) : ++t : e[t] === "." ? e.splice(t, 1) : ++t;
        return i + e.join("/");
      }
    );
    f.resolve = function(n, e, r) {
      return r || (e = c(e)), h(e) ? e : (r || (n = c(n)), (n = n.replace(/(?:\\/|^)[^/]+$/, "")).length ? c(n + "/" + e) : e);
    };
  }(path)), path;
}
var namespace, hasRequiredNamespace;
function requireNamespace() {
  if (hasRequiredNamespace) return namespace;
  hasRequiredNamespace = 1, namespace = i;
  var a = requireObject();
  ((i.prototype = Object.create(a.prototype)).constructor = i).className = "Namespace";
  var f = requireField(), h = requireUtil(), c = requireOneof(), d, n, e;
  i.fromJSON = function(s, u) {
    return new i(s, u.options).addJSON(u.nested);
  };
  function r(l, s) {
    if (l && l.length) {
      for (var u = {}, o = 0; o < l.length; ++o)
        u[l[o].name] = l[o].toJSON(s);
      return u;
    }
  }
  i.arrayToJSON = r, i.isReservedId = function(s, u) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (typeof s[o] != "string" && s[o][0] <= u && s[o][1] > u)
          return !0;
    }
    return !1;
  }, i.isReservedName = function(s, u) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (s[o] === u)
          return !0;
    }
    return !1;
  };
  function i(l, s) {
    a.call(this, l, s), this.nested = void 0, this._nestedArray = null, this._lookupCache = {}, this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
  }
  function t(l) {
    l._nestedArray = null, l._lookupCache = {};
    for (var s = l; s = s.parent; )
      s._lookupCache = {};
    return l;
  }
  return Object.defineProperty(i.prototype, "nestedArray", {
    get: function() {
      return this._nestedArray || (this._nestedArray = h.toArray(this.nested));
    }
  }), i.prototype.toJSON = function(s) {
    return h.toObject([
      "options",
      this.options,
      "nested",
      r(this.nestedArray, s)
    ]);
  }, i.prototype.addJSON = function(s) {
    var u = this;
    if (s)
      for (var o = Object.keys(s), p = 0, y; p < o.length; ++p)
        y = s[o[p]], u.add(
          // most to least likely
          (y.fields !== void 0 ? d.fromJSON : y.values !== void 0 ? e.fromJSON : y.methods !== void 0 ? n.fromJSON : y.id !== void 0 ? f.fromJSON : i.fromJSON)(o[p], y)
        );
    return this;
  }, i.prototype.get = function(s) {
    return this.nested && this.nested[s] || null;
  }, i.prototype.getEnum = function(s) {
    if (this.nested && this.nested[s] instanceof e)
      return this.nested[s].values;
    throw Error("no such enum: " + s);
  }, i.prototype.add = function(s) {
    if (!(s instanceof f && s.extend !== void 0 || s instanceof d || s instanceof c || s instanceof e || s instanceof n || s instanceof i))
      throw TypeError("object must be a valid nested object");
    if (!this.nested)
      this.nested = {};
    else {
      var u = this.get(s.name);
      if (u)
        if (u instanceof i && s instanceof i && !(u instanceof d || u instanceof n)) {
          for (var o = u.nestedArray, p = 0; p < o.length; ++p)
            s.add(o[p]);
          this.remove(u), this.nested || (this.nested = {}), s.setOptions(u.options, !0);
        } else
          throw Error("duplicate name '" + s.name + "' in " + this);
    }
    this.nested[s.name] = s, this instanceof d || this instanceof n || this instanceof e || this instanceof f || s._edition || (s._edition = s._defaultEdition), this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
    for (var y = this; y = y.parent; )
      y._needsRecursiveFeatureResolution = !0, y._needsRecursiveResolve = !0;
    return s.onAdd(this), t(this);
  }, i.prototype.remove = function(s) {
    if (!(s instanceof a))
      throw TypeError("object must be a ReflectionObject");
    if (s.parent !== this)
      throw Error(s + " is not a member of " + this);
    return delete this.nested[s.name], Object.keys(this.nested).length || (this.nested = void 0), s.onRemove(this), t(this);
  }, i.prototype.define = function(s, u) {
    if (h.isString(s))
      s = s.split(".");
    else if (!Array.isArray(s))
      throw TypeError("illegal path");
    if (s && s.length && s[0] === "")
      throw Error("path must be relative");
    for (var o = this; s.length > 0; ) {
      var p = s.shift();
      if (o.nested && o.nested[p]) {
        if (o = o.nested[p], !(o instanceof i))
          throw Error("path conflicts with non-namespace objects");
      } else
        o.add(o = new i(p));
    }
    return u && o.addJSON(u), o;
  }, i.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    this._resolveFeaturesRecursive(this._edition);
    var s = this.nestedArray, u = 0;
    for (this.resolve(); u < s.length; )
      s[u] instanceof i ? s[u++].resolveAll() : s[u++].resolve();
    return this._needsRecursiveResolve = !1, this;
  }, i.prototype._resolveFeaturesRecursive = function(s) {
    return this._needsRecursiveFeatureResolution ? (this._needsRecursiveFeatureResolution = !1, s = this._edition || s, a.prototype._resolveFeaturesRecursive.call(this, s), this.nestedArray.forEach((u) => {
      u._resolveFeaturesRecursive(s);
    }), this) : this;
  }, i.prototype.lookup = function(s, u, o) {
    if (typeof u == "boolean" ? (o = u, u = void 0) : u && !Array.isArray(u) && (u = [u]), h.isString(s) && s.length) {
      if (s === ".")
        return this.root;
      s = s.split(".");
    } else if (!s.length)
      return this;
    var p = s.join(".");
    if (s[0] === "")
      return this.root.lookup(s.slice(1), u);
    var y = this.root._fullyQualifiedObjects && this.root._fullyQualifiedObjects["." + p];
    if (y && (!u || u.indexOf(y.constructor) > -1) || (y = this._lookupImpl(s, p), y && (!u || u.indexOf(y.constructor) > -1)))
      return y;
    if (o)
      return null;
    for (var E = this; E.parent; ) {
      if (y = E.parent._lookupImpl(s, p), y && (!u || u.indexOf(y.constructor) > -1))
        return y;
      E = E.parent;
    }
    return null;
  }, i.prototype._lookupImpl = function(s, u) {
    if (Object.prototype.hasOwnProperty.call(this._lookupCache, u))
      return this._lookupCache[u];
    var o = this.get(s[0]), p = null;
    if (o)
      s.length === 1 ? p = o : o instanceof i && (s = s.slice(1), p = o._lookupImpl(s, s.join(".")));
    else
      for (var y = 0; y < this.nestedArray.length; ++y)
        this._nestedArray[y] instanceof i && (o = this._nestedArray[y]._lookupImpl(s, u)) && (p = o);
    return this._lookupCache[u] = p, p;
  }, i.prototype.lookupType = function(s) {
    var u = this.lookup(s, [d]);
    if (!u)
      throw Error("no such type: " + s);
    return u;
  }, i.prototype.lookupEnum = function(s) {
    var u = this.lookup(s, [e]);
    if (!u)
      throw Error("no such Enum '" + s + "' in " + this);
    return u;
  }, i.prototype.lookupTypeOrEnum = function(s) {
    var u = this.lookup(s, [d, e]);
    if (!u)
      throw Error("no such Type or Enum '" + s + "' in " + this);
    return u;
  }, i.prototype.lookupService = function(s) {
    var u = this.lookup(s, [n]);
    if (!u)
      throw Error("no such Service '" + s + "' in " + this);
    return u;
  }, i._configure = function(l, s, u) {
    d = l, n = s, e = u;
  }, namespace;
}
var mapfield, hasRequiredMapfield;
function requireMapfield() {
  if (hasRequiredMapfield) return mapfield;
  hasRequiredMapfield = 1, mapfield = c;
  var a = requireField();
  ((c.prototype = Object.create(a.prototype)).constructor = c).className = "MapField";
  var f = requireTypes(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (a.call(this, d, n, r, void 0, void 0, i, t), !h.isString(e))
      throw TypeError("keyType must be a string");
    this.keyType = e, this.resolvedKeyType = null, this.map = !0;
  }
  return c.fromJSON = function(n, e) {
    return new c(n, e.id, e.keyType, e.type, e.options, e.comment);
  }, c.prototype.toJSON = function(n) {
    var e = n ? !!n.keepComments : !1;
    return h.toObject([
      "keyType",
      this.keyType,
      "type",
      this.type,
      "id",
      this.id,
      "extend",
      this.extend,
      "options",
      this.options,
      "comment",
      e ? this.comment : void 0
    ]);
  }, c.prototype.resolve = function() {
    if (this.resolved)
      return this;
    if (f.mapKey[this.keyType] === void 0)
      throw Error("invalid key type: " + this.keyType);
    return a.prototype.resolve.call(this);
  }, c.d = function(n, e, r) {
    return typeof r == "function" ? r = h.decorateType(r).name : r && typeof r == "object" && (r = h.decorateEnum(r).name), function(t, l) {
      h.decorateType(t.constructor).add(new c(l, n, e, r));
    };
  }, mapfield;
}
var method, hasRequiredMethod;
function requireMethod() {
  if (hasRequiredMethod) return method;
  hasRequiredMethod = 1, method = h;
  var a = requireObject();
  ((h.prototype = Object.create(a.prototype)).constructor = h).className = "Method";
  var f = requireUtil();
  function h(c, d, n, e, r, i, t, l, s) {
    if (f.isObject(r) ? (t = r, r = i = void 0) : f.isObject(i) && (t = i, i = void 0), !(d === void 0 || f.isString(d)))
      throw TypeError("type must be a string");
    if (!f.isString(n))
      throw TypeError("requestType must be a string");
    if (!f.isString(e))
      throw TypeError("responseType must be a string");
    a.call(this, c, t), this.type = d || "rpc", this.requestType = n, this.requestStream = r ? !0 : void 0, this.responseType = e, this.responseStream = i ? !0 : void 0, this.resolvedRequestType = null, this.resolvedResponseType = null, this.comment = l, this.parsedOptions = s;
  }
  return h.fromJSON = function(d, n) {
    return new h(d, n.type, n.requestType, n.responseType, n.requestStream, n.responseStream, n.options, n.comment, n.parsedOptions);
  }, h.prototype.toJSON = function(d) {
    var n = d ? !!d.keepComments : !1;
    return f.toObject([
      "type",
      this.type !== "rpc" && /* istanbul ignore next */
      this.type || void 0,
      "requestType",
      this.requestType,
      "requestStream",
      this.requestStream,
      "responseType",
      this.responseType,
      "responseStream",
      this.responseStream,
      "options",
      this.options,
      "comment",
      n ? this.comment : void 0,
      "parsedOptions",
      this.parsedOptions
    ]);
  }, h.prototype.resolve = function() {
    return this.resolved ? this : (this.resolvedRequestType = this.parent.lookupType(this.requestType), this.resolvedResponseType = this.parent.lookupType(this.responseType), a.prototype.resolve.call(this));
  }, method;
}
var service, hasRequiredService;
function requireService() {
  if (hasRequiredService) return service;
  hasRequiredService = 1, service = d;
  var a = requireNamespace();
  ((d.prototype = Object.create(a.prototype)).constructor = d).className = "Service";
  var f = requireMethod(), h = requireUtil(), c = requireRpc();
  function d(e, r) {
    a.call(this, e, r), this.methods = {}, this._methodsArray = null;
  }
  d.fromJSON = function(r, i) {
    var t = new d(r, i.options);
    if (i.methods)
      for (var l = Object.keys(i.methods), s = 0; s < l.length; ++s)
        t.add(f.fromJSON(l[s], i.methods[l[s]]));
    return i.nested && t.addJSON(i.nested), i.edition && (t._edition = i.edition), t.comment = i.comment, t._defaultEdition = "proto3", t;
  }, d.prototype.toJSON = function(r) {
    var i = a.prototype.toJSON.call(this, r), t = r ? !!r.keepComments : !1;
    return h.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      i && i.options || void 0,
      "methods",
      a.arrayToJSON(this.methodsArray, r) || /* istanbul ignore next */
      {},
      "nested",
      i && i.nested || void 0,
      "comment",
      t ? this.comment : void 0
    ]);
  }, Object.defineProperty(d.prototype, "methodsArray", {
    get: function() {
      return this._methodsArray || (this._methodsArray = h.toArray(this.methods));
    }
  });
  function n(e) {
    return e._methodsArray = null, e;
  }
  return d.prototype.get = function(r) {
    return this.methods[r] || a.prototype.get.call(this, r);
  }, d.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    a.prototype.resolve.call(this);
    for (var r = this.methodsArray, i = 0; i < r.length; ++i)
      r[i].resolve();
    return this;
  }, d.prototype._resolveFeaturesRecursive = function(r) {
    return this._needsRecursiveFeatureResolution ? (r = this._edition || r, a.prototype._resolveFeaturesRecursive.call(this, r), this.methodsArray.forEach((i) => {
      i._resolveFeaturesRecursive(r);
    }), this) : this;
  }, d.prototype.add = function(r) {
    if (this.get(r.name))
      throw Error("duplicate name '" + r.name + "' in " + this);
    return r instanceof f ? (this.methods[r.name] = r, r.parent = this, n(this)) : a.prototype.add.call(this, r);
  }, d.prototype.remove = function(r) {
    if (r instanceof f) {
      if (this.methods[r.name] !== r)
        throw Error(r + " is not a member of " + this);
      return delete this.methods[r.name], r.parent = null, n(this);
    }
    return a.prototype.remove.call(this, r);
  }, d.prototype.create = function(r, i, t) {
    for (var l = new c.Service(r, i, t), s = 0, u; s < /* initializes */
    this.methodsArray.length; ++s) {
      var o = h.lcFirst((u = this._methodsArray[s]).resolve().name).replace(/[^$\\w_]/g, "");
      l[o] = h.codegen(["r", "c"], h.isReserved(o) ? o + "_" : o)("return this.rpcCall(m,q,s,r,c)")({
        m: u,
        q: u.resolvedRequestType.ctor,
        s: u.resolvedResponseType.ctor
      });
    }
    return l;
  }, service;
}
var message, hasRequiredMessage;
function requireMessage() {
  if (hasRequiredMessage) return message;
  hasRequiredMessage = 1, message = f;
  var a = requireMinimal();
  function f(h) {
    if (h)
      for (var c = Object.keys(h), d = 0; d < c.length; ++d) {
        var n = c[d];
        n !== "__proto__" && (this[n] = h[n]);
      }
  }
  return f.create = function(c) {
    return this.$type.create(c);
  }, f.encode = function(c, d) {
    return this.$type.encode(c, d);
  }, f.encodeDelimited = function(c, d) {
    return this.$type.encodeDelimited(c, d);
  }, f.decode = function(c) {
    return this.$type.decode(c);
  }, f.decodeDelimited = function(c) {
    return this.$type.decodeDelimited(c);
  }, f.verify = function(c) {
    return this.$type.verify(c);
  }, f.fromObject = function(c) {
    return this.$type.fromObject(c);
  }, f.toObject = function(c, d) {
    return this.$type.toObject(c, d);
  }, f.prototype.toJSON = function() {
    return this.$type.toObject(this, a.toJSONOptions);
  }, message;
}
var decoder_1, hasRequiredDecoder;
function requireDecoder() {
  if (hasRequiredDecoder) return decoder_1;
  hasRequiredDecoder = 1, decoder_1 = d;
  var a = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n) {
    return "missing required '" + n.name + "'";
  }
  function d(n) {
    for (var e = h.codegen(["r", "l", "e"], n.name + "$decode")("if(!(r instanceof Reader))")("r=Reader.create(r)")("var c=l===undefined?r.len:r.pos+l,m=new this.ctor" + (n.fieldsArray.filter(function(u) {
      return u.map;
    }).length ? ",k,value" : ""))("while(r.pos<c){")("var t=r.uint32()")("if(t===e)")("break")("switch(t>>>3){"), r = 0; r < /* initializes */
    n.fieldsArray.length; ++r) {
      var i = n._fieldsArray[r].resolve(), t = i.resolvedType instanceof a ? "int32" : i.type, l = "m" + h.safeProp(i.name);
      e("case %i: {", i.id), i.map ? (e("if(%s===util.emptyObject)", l)("%s={}", l)("var c2 = r.uint32()+r.pos"), f.defaults[i.keyType] !== void 0 ? e("k=%j", f.defaults[i.keyType]) : e("k=null"), f.defaults[t] !== void 0 ? e("value=%j", f.defaults[t]) : e("value=null"), e("while(r.pos<c2){")("var tag2=r.uint32()")("switch(tag2>>>3){")("case 1: k=r.%s(); break", i.keyType)("case 2:"), f.basic[t] === void 0 ? e("value=types[%i].decode(r,r.uint32())", r) : e("value=r.%s()", t), e("break")("default:")("r.skipType(tag2&7)")("break")("}")("}"), f.long[i.keyType] !== void 0 ? e('%s[typeof k==="object"?util.longToHash(k):k]=value', l) : e("%s[k]=value", l)) : i.repeated ? (e("if(!(%s&&%s.length))", l, l)("%s=[]", l), f.packed[t] !== void 0 && e("if((t&7)===2){")("var c2=r.uint32()+r.pos")("while(r.pos<c2)")("%s.push(r.%s())", l, t)("}else"), f.basic[t] === void 0 ? e(i.delimited ? "%s.push(types[%i].decode(r,undefined,((t&~7)|4)))" : "%s.push(types[%i].decode(r,r.uint32()))", l, r) : e("%s.push(r.%s())", l, t)) : f.basic[t] === void 0 ? e(i.delimited ? "%s=types[%i].decode(r,undefined,((t&~7)|4))" : "%s=types[%i].decode(r,r.uint32())", l, r) : e("%s=r.%s()", l, t), e("break")("}");
    }
    for (e("default:")("r.skipType(t&7)")("break")("}")("}"), r = 0; r < n._fieldsArray.length; ++r) {
      var s = n._fieldsArray[r];
      s.required && e("if(!m.hasOwnProperty(%j))", s.name)("throw util.ProtocolError(%j,{instance:m})", c(s));
    }
    return e("return m");
  }
  return decoder_1;
}
var verifier_1, hasRequiredVerifier;
function requireVerifier() {
  if (hasRequiredVerifier) return verifier_1;
  hasRequiredVerifier = 1, verifier_1 = n;
  var a = require_enum(), f = requireUtil();
  function h(e, r) {
    return e.name + ": " + r + (e.repeated && r !== "array" ? "[]" : e.map && r !== "object" ? "{k:" + e.keyType + "}" : "") + " expected";
  }
  function c(e, r, i, t) {
    if (r.resolvedType)
      if (r.resolvedType instanceof a) {
        e("switch(%s){", t)("default:")("return%j", h(r, "enum value"));
        for (var l = Object.keys(r.resolvedType.values), s = 0; s < l.length; ++s) e("case %i:", r.resolvedType.values[l[s]]);
        e("break")("}");
      } else
        e("{")("var e=types[%i].verify(%s);", i, t)("if(e)")("return%j+e", r.name + ".")("}");
    else
      switch (r.type) {
        case "int32":
        case "uint32":
        case "sint32":
        case "fixed32":
        case "sfixed32":
          e("if(!util.isInteger(%s))", t)("return%j", h(r, "integer"));
          break;
        case "int64":
        case "uint64":
        case "sint64":
        case "fixed64":
        case "sfixed64":
          e("if(!util.isInteger(%s)&&!(%s&&util.isInteger(%s.low)&&util.isInteger(%s.high)))", t, t, t, t)("return%j", h(r, "integer|Long"));
          break;
        case "float":
        case "double":
          e('if(typeof %s!=="number")', t)("return%j", h(r, "number"));
          break;
        case "bool":
          e('if(typeof %s!=="boolean")', t)("return%j", h(r, "boolean"));
          break;
        case "string":
          e("if(!util.isString(%s))", t)("return%j", h(r, "string"));
          break;
        case "bytes":
          e('if(!(%s&&typeof %s.length==="number"||util.isString(%s)))', t, t, t)("return%j", h(r, "buffer"));
          break;
      }
    return e;
  }
  function d(e, r, i) {
    switch (r.keyType) {
      case "int32":
      case "uint32":
      case "sint32":
      case "fixed32":
      case "sfixed32":
        e("if(!util.key32Re.test(%s))", i)("return%j", h(r, "integer key"));
        break;
      case "int64":
      case "uint64":
      case "sint64":
      case "fixed64":
      case "sfixed64":
        e("if(!util.key64Re.test(%s))", i)("return%j", h(r, "integer|Long key"));
        break;
      case "bool":
        e("if(!util.key2Re.test(%s))", i)("return%j", h(r, "boolean key"));
        break;
    }
    return e;
  }
  function n(e) {
    var r = f.codegen(["m"], e.name + "$verify")('if(typeof m!=="object"||m===null)')("return%j", "object expected"), i = e.oneofsArray, t = {};
    i.length && r("var p={}");
    for (var l = 0; l < /* initializes */
    e.fieldsArray.length; ++l) {
      var s = e._fieldsArray[l].resolve(), u = "m" + f.safeProp(s.name);
      if (s.optional && r("if(%s!=null&&m.hasOwnProperty(%j)){", u, s.name), s.map)
        r("if(!util.isObject(%s))", u)("return%j", h(s, "object"))("var k=Object.keys(%s)", u)("for(var i=0;i<k.length;++i){"), d(r, s, "k[i]"), c(r, s, l, u + "[k[i]]")("}");
      else if (s.repeated)
        r("if(!Array.isArray(%s))", u)("return%j", h(s, "array"))("for(var i=0;i<%s.length;++i){", u), c(r, s, l, u + "[i]")("}");
      else {
        if (s.partOf) {
          var o = f.safeProp(s.partOf.name);
          t[s.partOf.name] === 1 && r("if(p%s===1)", o)("return%j", s.partOf.name + ": multiple values"), t[s.partOf.name] = 1, r("p%s=1", o);
        }
        c(r, s, l, u);
      }
      s.optional && r("}");
    }
    return r("return null");
  }
  return verifier_1;
}
var converter = {}, hasRequiredConverter;
function requireConverter() {
  return hasRequiredConverter || (hasRequiredConverter = 1, function(a) {
    var f = a, h = require_enum(), c = requireUtil();
    function d(e, r, i, t) {
      var l = !1;
      if (r.resolvedType)
        if (r.resolvedType instanceof h) {
          e("switch(d%s){", t);
          for (var s = r.resolvedType.values, u = Object.keys(s), o = 0; o < u.length; ++o)
            s[u[o]] === r.typeDefault && !l && (e("default:")('if(typeof(d%s)==="number"){m%s=d%s;break}', t, t, t), r.repeated || e("break"), l = !0), e("case%j:", u[o])("case %i:", s[u[o]])("m%s=%j", t, s[u[o]])("break");
          e("}");
        } else e('if(typeof d%s!=="object")', t)("throw TypeError(%j)", r.fullName + ": object expected")("m%s=types[%i].fromObject(d%s)", t, i, t);
      else {
        var p = !1;
        switch (r.type) {
          case "double":
          case "float":
            e("m%s=Number(d%s)", t, t);
            break;
          case "uint32":
          case "fixed32":
            e("m%s=d%s>>>0", t, t);
            break;
          case "int32":
          case "sint32":
          case "sfixed32":
            e("m%s=d%s|0", t, t);
            break;
          case "uint64":
            p = !0;
          // eslint-disable-next-line no-fallthrough
          case "int64":
          case "sint64":
          case "fixed64":
          case "sfixed64":
            e("if(util.Long)")("(m%s=util.Long.fromValue(d%s)).unsigned=%j", t, t, p)('else if(typeof d%s==="string")', t)("m%s=parseInt(d%s,10)", t, t)('else if(typeof d%s==="number")', t)("m%s=d%s", t, t)('else if(typeof d%s==="object")', t)("m%s=new util.LongBits(d%s.low>>>0,d%s.high>>>0).toNumber(%s)", t, t, t, p ? "true" : "");
            break;
          case "bytes":
            e('if(typeof d%s==="string")', t)("util.base64.decode(d%s,m%s=util.newBuffer(util.base64.length(d%s)),0)", t, t, t)("else if(d%s.length >= 0)", t)("m%s=d%s", t, t);
            break;
          case "string":
            e("m%s=String(d%s)", t, t);
            break;
          case "bool":
            e("m%s=Boolean(d%s)", t, t);
            break;
        }
      }
      return e;
    }
    f.fromObject = function(r) {
      var i = r.fieldsArray, t = c.codegen(["d"], r.name + "$fromObject")("if(d instanceof this.ctor)")("return d");
      if (!i.length) return t("return new this.ctor");
      t("var m=new this.ctor");
      for (var l = 0; l < i.length; ++l) {
        var s = i[l].resolve(), u = c.safeProp(s.name);
        s.map ? (t("if(d%s){", u)('if(typeof d%s!=="object")', u)("throw TypeError(%j)", s.fullName + ": object expected")("m%s={}", u)("for(var ks=Object.keys(d%s),i=0;i<ks.length;++i){", u), d(
          t,
          s,
          /* not sorted */
          l,
          u + "[ks[i]]"
        )("}")("}")) : s.repeated ? (t("if(d%s){", u)("if(!Array.isArray(d%s))", u)("throw TypeError(%j)", s.fullName + ": array expected")("m%s=[]", u)("for(var i=0;i<d%s.length;++i){", u), d(
          t,
          s,
          /* not sorted */
          l,
          u + "[i]"
        )("}")("}")) : (s.resolvedType instanceof h || t("if(d%s!=null){", u), d(
          t,
          s,
          /* not sorted */
          l,
          u
        ), s.resolvedType instanceof h || t("}"));
      }
      return t("return m");
    };
    function n(e, r, i, t) {
      if (r.resolvedType)
        r.resolvedType instanceof h ? e("d%s=o.enums===String?(types[%i].values[m%s]===undefined?m%s:types[%i].values[m%s]):m%s", t, i, t, t, i, t, t) : e("d%s=types[%i].toObject(m%s,o)", t, i, t);
      else {
        var l = !1;
        switch (r.type) {
          case "double":
          case "float":
            e("d%s=o.json&&!isFinite(m%s)?String(m%s):m%s", t, t, t, t);
            break;
          case "uint64":
            l = !0;
          // eslint-disable-next-line no-fallthrough
          case "int64":
          case "sint64":
          case "fixed64":
          case "sfixed64":
            e('if(typeof m%s==="number")', t)("d%s=o.longs===String?String(m%s):m%s", t, t, t)("else")("d%s=o.longs===String?util.Long.prototype.toString.call(m%s):o.longs===Number?new util.LongBits(m%s.low>>>0,m%s.high>>>0).toNumber(%s):m%s", t, t, t, t, l ? "true" : "", t);
            break;
          case "bytes":
            e("d%s=o.bytes===String?util.base64.encode(m%s,0,m%s.length):o.bytes===Array?Array.prototype.slice.call(m%s):m%s", t, t, t, t, t);
            break;
          default:
            e("d%s=m%s", t, t);
            break;
        }
      }
      return e;
    }
    f.toObject = function(r) {
      var i = r.fieldsArray.slice().sort(c.compareFieldsById);
      if (!i.length)
        return c.codegen()("return {}");
      for (var t = c.codegen(["m", "o"], r.name + "$toObject")("if(!o)")("o={}")("var d={}"), l = [], s = [], u = [], o = 0; o < i.length; ++o)
        i[o].partOf || (i[o].resolve().repeated ? l : i[o].map ? s : u).push(i[o]);
      if (l.length) {
        for (t("if(o.arrays||o.defaults){"), o = 0; o < l.length; ++o) t("d%s=[]", c.safeProp(l[o].name));
        t("}");
      }
      if (s.length) {
        for (t("if(o.objects||o.defaults){"), o = 0; o < s.length; ++o) t("d%s={}", c.safeProp(s[o].name));
        t("}");
      }
      if (u.length) {
        for (t("if(o.defaults){"), o = 0; o < u.length; ++o) {
          var p = u[o], y = c.safeProp(p.name);
          if (p.resolvedType instanceof h) t("d%s=o.enums===String?%j:%j", y, p.resolvedType.valuesById[p.typeDefault], p.typeDefault);
          else if (p.long) t("if(util.Long){")("var n=new util.Long(%i,%i,%j)", p.typeDefault.low, p.typeDefault.high, p.typeDefault.unsigned)("d%s=o.longs===String?n.toString():o.longs===Number?n.toNumber():n", y)("}else")("d%s=o.longs===String?%j:%i", y, p.typeDefault.toString(), p.typeDefault.toNumber());
          else if (p.bytes) {
            var E = "[" + Array.prototype.slice.call(p.typeDefault).join(",") + "]";
            t("if(o.bytes===String)d%s=%j", y, String.fromCharCode.apply(String, p.typeDefault))("else{")("d%s=%s", y, E)("if(o.bytes!==Array)d%s=util.newBuffer(d%s)", y, y)("}");
          } else t("d%s=%j", y, p.typeDefault);
        }
        t("}");
      }
      var v = !1;
      for (o = 0; o < i.length; ++o) {
        var p = i[o], m = r._fieldsArray.indexOf(p), y = c.safeProp(p.name);
        p.map ? (v || (v = !0, t("var ks2")), t("if(m%s&&(ks2=Object.keys(m%s)).length){", y, y)("d%s={}", y)("for(var j=0;j<ks2.length;++j){"), n(
          t,
          p,
          /* sorted */
          m,
          y + "[ks2[j]]"
        )("}")) : p.repeated ? (t("if(m%s&&m%s.length){", y, y)("d%s=[]", y)("for(var j=0;j<m%s.length;++j){", y), n(
          t,
          p,
          /* sorted */
          m,
          y + "[j]"
        )("}")) : (t("if(m%s!=null&&m.hasOwnProperty(%j)){", y, p.name), n(
          t,
          p,
          /* sorted */
          m,
          y
        ), p.partOf && t("if(o.oneofs)")("d%s=%j", c.safeProp(p.partOf.name), p.name)), t("}");
      }
      return t("return d");
    };
  }(converter)), converter;
}
var wrappers = {}, hasRequiredWrappers;
function requireWrappers() {
  return hasRequiredWrappers || (hasRequiredWrappers = 1, function(a) {
    var f = a, h = requireMessage();
    f[".google.protobuf.Any"] = {
      fromObject: function(c) {
        if (c && c["@type"]) {
          var d = c["@type"].substring(c["@type"].lastIndexOf("/") + 1), n = this.lookup(d);
          if (n) {
            var e = c["@type"].charAt(0) === "." ? c["@type"].slice(1) : c["@type"];
            return e.indexOf("/") === -1 && (e = "/" + e), this.create({
              type_url: e,
              value: n.encode(n.fromObject(c)).finish()
            });
          }
        }
        return this.fromObject(c);
      },
      toObject: function(c, d) {
        var n = "type.googleapis.com/", e = "", r = "";
        if (d && d.json && c.type_url && c.value) {
          r = c.type_url.substring(c.type_url.lastIndexOf("/") + 1), e = c.type_url.substring(0, c.type_url.lastIndexOf("/") + 1);
          var i = this.lookup(r);
          i && (c = i.decode(c.value));
        }
        if (!(c instanceof this.ctor) && c instanceof h) {
          var t = c.$type.toObject(c, d), l = c.$type.fullName[0] === "." ? c.$type.fullName.slice(1) : c.$type.fullName;
          return e === "" && (e = n), r = e + l, t["@type"] = r, t;
        }
        return this.toObject(c, d);
      }
    };
  }(wrappers)), wrappers;
}
var type, hasRequiredType;
function requireType() {
  if (hasRequiredType) return type;
  hasRequiredType = 1, type = y;
  var a = requireNamespace();
  ((y.prototype = Object.create(a.prototype)).constructor = y).className = "Type";
  var f = require_enum(), h = requireOneof(), c = requireField(), d = requireMapfield(), n = requireService(), e = requireMessage(), r = requireReader(), i = requireWriter(), t = requireUtil(), l = requireEncoder(), s = requireDecoder(), u = requireVerifier(), o = requireConverter(), p = requireWrappers();
  function y(v, m) {
    v = v.replace(/\\W/g, ""), a.call(this, v, m), this.fields = {}, this.oneofs = void 0, this.extensions = void 0, this.reserved = void 0, this.group = void 0, this._fieldsById = null, this._fieldsArray = null, this._oneofsArray = null, this._ctor = null;
  }
  Object.defineProperties(y.prototype, {
    /**
     * Message fields by id.
     * @name Type#fieldsById
     * @type {Object.<number,Field>}
     * @readonly
     */
    fieldsById: {
      get: function() {
        if (this._fieldsById)
          return this._fieldsById;
        this._fieldsById = {};
        for (var v = Object.keys(this.fields), m = 0; m < v.length; ++m) {
          var _ = this.fields[v[m]], A = _.id;
          if (this._fieldsById[A])
            throw Error("duplicate id " + A + " in " + this);
          this._fieldsById[A] = _;
        }
        return this._fieldsById;
      }
    },
    /**
     * Fields of this message as an array for iteration.
     * @name Type#fieldsArray
     * @type {Field[]}
     * @readonly
     */
    fieldsArray: {
      get: function() {
        return this._fieldsArray || (this._fieldsArray = t.toArray(this.fields));
      }
    },
    /**
     * Oneofs of this message as an array for iteration.
     * @name Type#oneofsArray
     * @type {OneOf[]}
     * @readonly
     */
    oneofsArray: {
      get: function() {
        return this._oneofsArray || (this._oneofsArray = t.toArray(this.oneofs));
      }
    },
    /**
     * The registered constructor, if any registered, otherwise a generic constructor.
     * Assigning a function replaces the internal constructor. If the function does not extend {@link Message} yet, its prototype will be setup accordingly and static methods will be populated. If it already extends {@link Message}, it will just replace the internal constructor.
     * @name Type#ctor
     * @type {Constructor<{}>}
     */
    ctor: {
      get: function() {
        return this._ctor || (this.ctor = y.generateConstructor(this)());
      },
      set: function(v) {
        var m = v.prototype;
        m instanceof e || ((v.prototype = new e()).constructor = v, t.merge(v.prototype, m)), v.$type = v.prototype.$type = this, t.merge(v, e, !0), this._ctor = v;
        for (var _ = 0; _ < /* initializes */
        this.fieldsArray.length; ++_)
          this._fieldsArray[_].resolve();
        var A = {};
        for (_ = 0; _ < /* initializes */
        this.oneofsArray.length; ++_)
          A[this._oneofsArray[_].resolve().name] = {
            get: t.oneOfGetter(this._oneofsArray[_].oneof),
            set: t.oneOfSetter(this._oneofsArray[_].oneof)
          };
        _ && Object.defineProperties(v.prototype, A);
      }
    }
  }), y.generateConstructor = function(m) {
    for (var _ = t.codegen(["p"], m.name), A = 0, I; A < m.fieldsArray.length; ++A)
      (I = m._fieldsArray[A]).map ? _("this%s={}", t.safeProp(I.name)) : I.repeated && _("this%s=[]", t.safeProp(I.name));
    return _("if(p)for(var ks=Object.keys(p),i=0;i<ks.length;++i)if(p[ks[i]]!=null)")("this[ks[i]]=p[ks[i]]");
  };
  function E(v) {
    return v._fieldsById = v._fieldsArray = v._oneofsArray = null, delete v.encode, delete v.decode, delete v.verify, v;
  }
  return y.fromJSON = function(m, _) {
    var A = new y(m, _.options);
    A.extensions = _.extensions, A.reserved = _.reserved;
    for (var I = Object.keys(_.fields), C = 0; C < I.length; ++C)
      A.add(
        (typeof _.fields[I[C]].keyType < "u" ? d.fromJSON : c.fromJSON)(I[C], _.fields[I[C]])
      );
    if (_.oneofs)
      for (I = Object.keys(_.oneofs), C = 0; C < I.length; ++C)
        A.add(h.fromJSON(I[C], _.oneofs[I[C]]));
    if (_.nested)
      for (I = Object.keys(_.nested), C = 0; C < I.length; ++C) {
        var j = _.nested[I[C]];
        A.add(
          // most to least likely
          (j.id !== void 0 ? c.fromJSON : j.fields !== void 0 ? y.fromJSON : j.values !== void 0 ? f.fromJSON : j.methods !== void 0 ? n.fromJSON : a.fromJSON)(I[C], j)
        );
      }
    return _.extensions && _.extensions.length && (A.extensions = _.extensions), _.reserved && _.reserved.length && (A.reserved = _.reserved), _.group && (A.group = !0), _.comment && (A.comment = _.comment), _.edition && (A._edition = _.edition), A._defaultEdition = "proto3", A;
  }, y.prototype.toJSON = function(m) {
    var _ = a.prototype.toJSON.call(this, m), A = m ? !!m.keepComments : !1;
    return t.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      _ && _.options || void 0,
      "oneofs",
      a.arrayToJSON(this.oneofsArray, m),
      "fields",
      a.arrayToJSON(this.fieldsArray.filter(function(I) {
        return !I.declaringField;
      }), m) || {},
      "extensions",
      this.extensions && this.extensions.length ? this.extensions : void 0,
      "reserved",
      this.reserved && this.reserved.length ? this.reserved : void 0,
      "group",
      this.group || void 0,
      "nested",
      _ && _.nested || void 0,
      "comment",
      A ? this.comment : void 0
    ]);
  }, y.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    a.prototype.resolveAll.call(this);
    var m = this.oneofsArray;
    for (A = 0; A < m.length; )
      m[A++].resolve();
    for (var _ = this.fieldsArray, A = 0; A < _.length; )
      _[A++].resolve();
    return this;
  }, y.prototype._resolveFeaturesRecursive = function(m) {
    return this._needsRecursiveFeatureResolution ? (m = this._edition || m, a.prototype._resolveFeaturesRecursive.call(this, m), this.oneofsArray.forEach((_) => {
      _._resolveFeatures(m);
    }), this.fieldsArray.forEach((_) => {
      _._resolveFeatures(m);
    }), this) : this;
  }, y.prototype.get = function(m) {
    return this.fields[m] || this.oneofs && this.oneofs[m] || this.nested && this.nested[m] || null;
  }, y.prototype.add = function(m) {
    if (this.get(m.name))
      throw Error("duplicate name '" + m.name + "' in " + this);
    if (m instanceof c && m.extend === void 0) {
      if (this._fieldsById ? (
        /* istanbul ignore next */
        this._fieldsById[m.id]
      ) : this.fieldsById[m.id])
        throw Error("duplicate id " + m.id + " in " + this);
      if (this.isReservedId(m.id))
        throw Error("id " + m.id + " is reserved in " + this);
      if (this.isReservedName(m.name))
        throw Error("name '" + m.name + "' is reserved in " + this);
      return m.parent && m.parent.remove(m), this.fields[m.name] = m, m.message = this, m.onAdd(this), E(this);
    }
    return m instanceof h ? (this.oneofs || (this.oneofs = {}), this.oneofs[m.name] = m, m.onAdd(this), E(this)) : a.prototype.add.call(this, m);
  }, y.prototype.remove = function(m) {
    if (m instanceof c && m.extend === void 0) {
      if (!this.fields || this.fields[m.name] !== m)
        throw Error(m + " is not a member of " + this);
      return delete this.fields[m.name], m.parent = null, m.onRemove(this), E(this);
    }
    if (m instanceof h) {
      if (!this.oneofs || this.oneofs[m.name] !== m)
        throw Error(m + " is not a member of " + this);
      return delete this.oneofs[m.name], m.parent = null, m.onRemove(this), E(this);
    }
    return a.prototype.remove.call(this, m);
  }, y.prototype.isReservedId = function(m) {
    return a.isReservedId(this.reserved, m);
  }, y.prototype.isReservedName = function(m) {
    return a.isReservedName(this.reserved, m);
  }, y.prototype.create = function(m) {
    return new this.ctor(m);
  }, y.prototype.setup = function() {
    for (var m = this.fullName, _ = [], A = 0; A < /* initializes */
    this.fieldsArray.length; ++A)
      _.push(this._fieldsArray[A].resolve().resolvedType);
    this.encode = l(this)({
      Writer: i,
      types: _,
      util: t
    }), this.decode = s(this)({
      Reader: r,
      types: _,
      util: t
    }), this.verify = u(this)({
      types: _,
      util: t
    }), this.fromObject = o.fromObject(this)({
      types: _,
      util: t
    }), this.toObject = o.toObject(this)({
      types: _,
      util: t
    });
    var I = p[m];
    if (I) {
      var C = Object.create(this);
      C.fromObject = this.fromObject, this.fromObject = I.fromObject.bind(C), C.toObject = this.toObject, this.toObject = I.toObject.bind(C);
    }
    return this;
  }, y.prototype.encode = function(m, _) {
    return this.setup().encode(m, _);
  }, y.prototype.encodeDelimited = function(m, _) {
    return this.encode(m, _ && _.len ? _.fork() : _).ldelim();
  }, y.prototype.decode = function(m, _) {
    return this.setup().decode(m, _);
  }, y.prototype.decodeDelimited = function(m) {
    return m instanceof r || (m = r.create(m)), this.decode(m, m.uint32());
  }, y.prototype.verify = function(m) {
    return this.setup().verify(m);
  }, y.prototype.fromObject = function(m) {
    return this.setup().fromObject(m);
  }, y.prototype.toObject = function(m, _) {
    return this.setup().toObject(m, _);
  }, y.d = function(m) {
    return function(A) {
      t.decorateType(A, m);
    };
  }, type;
}
var root$1, hasRequiredRoot;
function requireRoot() {
  if (hasRequiredRoot) return root$1;
  hasRequiredRoot = 1, root$1 = i;
  var a = requireNamespace();
  ((i.prototype = Object.create(a.prototype)).constructor = i).className = "Root";
  var f = requireField(), h = require_enum(), c = requireOneof(), d = requireUtil(), n, e, r;
  function i(u) {
    a.call(this, "", u), this.deferred = [], this.files = [], this._edition = "proto2", this._fullyQualifiedObjects = {};
  }
  i.fromJSON = function(o, p) {
    return p || (p = new i()), o.options && p.setOptions(o.options), p.addJSON(o.nested).resolveAll();
  }, i.prototype.resolvePath = d.path.resolve, i.prototype.fetch = d.fetch;
  function t() {
  }
  i.prototype.load = function u(o, p, y) {
    typeof p == "function" && (y = p, p = void 0);
    var E = this;
    if (!y)
      return d.asPromise(u, E, o, p);
    var v = y === t;
    function m(B, L) {
      if (y) {
        if (v)
          throw B;
        L && L.resolveAll();
        var S = y;
        y = null, S(B, L);
      }
    }
    function _(B) {
      var L = B.lastIndexOf("google/protobuf/");
      if (L > -1) {
        var S = B.substring(L);
        if (S in r) return S;
      }
      return null;
    }
    function A(B, L) {
      try {
        if (d.isString(L) && L.charAt(0) === "{" && (L = JSON.parse(L)), !d.isString(L))
          E.setOptions(L.options).addJSON(L.nested);
        else {
          e.filename = B;
          var S = e(L, E, p), J, M = 0;
          if (S.imports)
            for (; M < S.imports.length; ++M)
              (J = _(S.imports[M]) || E.resolvePath(B, S.imports[M])) && I(J);
          if (S.weakImports)
            for (M = 0; M < S.weakImports.length; ++M)
              (J = _(S.weakImports[M]) || E.resolvePath(B, S.weakImports[M])) && I(J, !0);
        }
      } catch (T) {
        m(T);
      }
      !v && !C && m(null, E);
    }
    function I(B, L) {
      if (B = _(B) || B, !(E.files.indexOf(B) > -1)) {
        if (E.files.push(B), B in r) {
          v ? A(B, r[B]) : (++C, setTimeout(function() {
            --C, A(B, r[B]);
          }));
          return;
        }
        if (v) {
          var S;
          try {
            S = d.fs.readFileSync(B).toString("utf8");
          } catch (J) {
            L || m(J);
            return;
          }
          A(B, S);
        } else
          ++C, E.fetch(B, function(J, M) {
            if (--C, !!y) {
              if (J) {
                L ? C || m(null, E) : m(J);
                return;
              }
              A(B, M);
            }
          });
      }
    }
    var C = 0;
    d.isString(o) && (o = [o]);
    for (var j = 0, K; j < o.length; ++j)
      (K = E.resolvePath("", o[j])) && I(K);
    return v ? (E.resolveAll(), E) : (C || m(null, E), E);
  }, i.prototype.loadSync = function(o, p) {
    if (!d.isNode)
      throw Error("not supported");
    return this.load(o, p, t);
  }, i.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    if (this.deferred.length)
      throw Error("unresolvable extensions: " + this.deferred.map(function(o) {
        return "'extend " + o.extend + "' in " + o.parent.fullName;
      }).join(", "));
    return a.prototype.resolveAll.call(this);
  };
  var l = /^[A-Z]/;
  function s(u, o) {
    var p = o.parent.lookup(o.extend);
    if (p) {
      var y = new f(o.fullName, o.id, o.type, o.rule, void 0, o.options);
      return p.get(y.name) || (y.declaringField = o, o.extensionField = y, p.add(y)), !0;
    }
    return !1;
  }
  return i.prototype._handleAdd = function(o) {
    if (o instanceof f)
      /* an extension field (implies not part of a oneof) */
      o.extend !== void 0 && /* not already handled */
      !o.extensionField && (s(this, o) || this.deferred.push(o));
    else if (o instanceof h)
      l.test(o.name) && (o.parent[o.name] = o.values);
    else if (!(o instanceof c)) {
      if (o instanceof n)
        for (var p = 0; p < this.deferred.length; )
          s(this, this.deferred[p]) ? this.deferred.splice(p, 1) : ++p;
      for (var y = 0; y < /* initializes */
      o.nestedArray.length; ++y)
        this._handleAdd(o._nestedArray[y]);
      l.test(o.name) && (o.parent[o.name] = o);
    }
    (o instanceof n || o instanceof h || o instanceof f) && (this._fullyQualifiedObjects[o.fullName] = o);
  }, i.prototype._handleRemove = function(o) {
    if (o instanceof f) {
      if (
        /* an extension field */
        o.extend !== void 0
      )
        if (
          /* already handled */
          o.extensionField
        )
          o.extensionField.parent.remove(o.extensionField), o.extensionField = null;
        else {
          var p = this.deferred.indexOf(o);
          p > -1 && this.deferred.splice(p, 1);
        }
    } else if (o instanceof h)
      l.test(o.name) && delete o.parent[o.name];
    else if (o instanceof a) {
      for (var y = 0; y < /* initializes */
      o.nestedArray.length; ++y)
        this._handleRemove(o._nestedArray[y]);
      l.test(o.name) && delete o.parent[o.name];
    }
    delete this._fullyQualifiedObjects[o.fullName];
  }, i._configure = function(u, o, p) {
    n = u, e = o, r = p;
  }, root$1;
}
var hasRequiredUtil;
function requireUtil() {
  if (hasRequiredUtil) return util.exports;
  hasRequiredUtil = 1;
  var a = util.exports = requireMinimal(), f = requireRoots(), h, c;
  a.codegen = requireCodegen(), a.fetch = requireFetch(), a.path = requirePath(), a.fs = a.inquire("fs"), a.toArray = function(t) {
    if (t) {
      for (var l = Object.keys(t), s = new Array(l.length), u = 0; u < l.length; )
        s[u] = t[l[u++]];
      return s;
    }
    return [];
  }, a.toObject = function(t) {
    for (var l = {}, s = 0; s < t.length; ) {
      var u = t[s++], o = t[s++];
      o !== void 0 && (l[u] = o);
    }
    return l;
  };
  var d = /\\\\/g, n = /"/g;
  a.isReserved = function(t) {
    return /^(?:do|if|in|for|let|new|try|var|case|else|enum|eval|false|null|this|true|void|with|break|catch|class|const|super|throw|while|yield|delete|export|import|public|return|static|switch|typeof|default|extends|finally|package|private|continue|debugger|function|arguments|interface|protected|implements|instanceof)$/.test(t);
  }, a.safeProp = function(t) {
    return !/^[$\\w_]+$/.test(t) || a.isReserved(t) ? '["' + t.replace(d, "\\\\\\\\").replace(n, '\\\\"') + '"]' : "." + t;
  }, a.ucFirst = function(t) {
    return t.charAt(0).toUpperCase() + t.substring(1);
  };
  var e = /_([a-z])/g;
  a.camelCase = function(t) {
    return t.substring(0, 1) + t.substring(1).replace(e, function(l, s) {
      return s.toUpperCase();
    });
  }, a.compareFieldsById = function(t, l) {
    return t.id - l.id;
  }, a.decorateType = function(t, l) {
    if (t.$type)
      return l && t.$type.name !== l && (a.decorateRoot.remove(t.$type), t.$type.name = l, a.decorateRoot.add(t.$type)), t.$type;
    h || (h = requireType());
    var s = new h(l || t.name);
    return a.decorateRoot.add(s), s.ctor = t, Object.defineProperty(t, "$type", { value: s, enumerable: !1 }), Object.defineProperty(t.prototype, "$type", { value: s, enumerable: !1 }), s;
  };
  var r = 0;
  return a.decorateEnum = function(t) {
    if (t.$type)
      return t.$type;
    c || (c = require_enum());
    var l = new c("Enum" + r++, t);
    return a.decorateRoot.add(l), Object.defineProperty(t, "$type", { value: l, enumerable: !1 }), l;
  }, a.setProperty = function(t, l, s, u) {
    function o(p, y, E) {
      var v = y.shift();
      if (v === "__proto__" || v === "prototype")
        return p;
      if (y.length > 0)
        p[v] = o(p[v] || {}, y, E);
      else {
        var m = p[v];
        if (m && u)
          return p;
        m && (E = [].concat(m).concat(E)), p[v] = E;
      }
      return p;
    }
    if (typeof t != "object")
      throw TypeError("dst must be an object");
    if (!l)
      throw TypeError("path must be specified");
    return l = l.split("."), o(t, l, s);
  }, Object.defineProperty(a, "decorateRoot", {
    get: function() {
      return f.decorated || (f.decorated = new (requireRoot())());
    }
  }), util.exports;
}
var hasRequiredTypes;
function requireTypes() {
  return hasRequiredTypes || (hasRequiredTypes = 1, function(a) {
    var f = a, h = requireUtil(), c = [
      "double",
      // 0
      "float",
      // 1
      "int32",
      // 2
      "uint32",
      // 3
      "sint32",
      // 4
      "fixed32",
      // 5
      "sfixed32",
      // 6
      "int64",
      // 7
      "uint64",
      // 8
      "sint64",
      // 9
      "fixed64",
      // 10
      "sfixed64",
      // 11
      "bool",
      // 12
      "string",
      // 13
      "bytes"
      // 14
    ];
    function d(n, e) {
      var r = 0, i = {};
      for (e |= 0; r < n.length; ) i[c[r + e]] = n[r++];
      return i;
    }
    f.basic = d([
      /* double   */
      1,
      /* float    */
      5,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0,
      /* string   */
      2,
      /* bytes    */
      2
    ]), f.defaults = d([
      /* double   */
      0,
      /* float    */
      0,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      0,
      /* sfixed32 */
      0,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      0,
      /* sfixed64 */
      0,
      /* bool     */
      !1,
      /* string   */
      "",
      /* bytes    */
      h.emptyArray,
      /* message  */
      null
    ]), f.long = d([
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1
    ], 7), f.mapKey = d([
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0,
      /* string   */
      2
    ], 2), f.packed = d([
      /* double   */
      1,
      /* float    */
      5,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0
    ]);
  }(types)), types;
}
var field, hasRequiredField;
function requireField() {
  if (hasRequiredField) return field;
  hasRequiredField = 1, field = e;
  var a = requireObject();
  ((e.prototype = Object.create(a.prototype)).constructor = e).className = "Field";
  var f = require_enum(), h = requireTypes(), c = requireUtil(), d, n = /^required|optional|repeated$/;
  e.fromJSON = function(i, t) {
    var l = new e(i, t.id, t.type, t.rule, t.extend, t.options, t.comment);
    return t.edition && (l._edition = t.edition), l._defaultEdition = "proto3", l;
  };
  function e(r, i, t, l, s, u, o) {
    if (c.isObject(l) ? (o = s, u = l, l = s = void 0) : c.isObject(s) && (o = u, u = s, s = void 0), a.call(this, r, u), !c.isInteger(i) || i < 0)
      throw TypeError("id must be a non-negative integer");
    if (!c.isString(t))
      throw TypeError("type must be a string");
    if (l !== void 0 && !n.test(l = l.toString().toLowerCase()))
      throw TypeError("rule must be a string rule");
    if (s !== void 0 && !c.isString(s))
      throw TypeError("extend must be a string");
    l === "proto3_optional" && (l = "optional"), this.rule = l && l !== "optional" ? l : void 0, this.type = t, this.id = i, this.extend = s || void 0, this.repeated = l === "repeated", this.map = !1, this.message = null, this.partOf = null, this.typeDefault = null, this.defaultValue = null, this.long = c.Long ? h.long[t] !== void 0 : (
      /* istanbul ignore next */
      !1
    ), this.bytes = t === "bytes", this.resolvedType = null, this.extensionField = null, this.declaringField = null, this.comment = o;
  }
  return Object.defineProperty(e.prototype, "required", {
    get: function() {
      return this._features.field_presence === "LEGACY_REQUIRED";
    }
  }), Object.defineProperty(e.prototype, "optional", {
    get: function() {
      return !this.required;
    }
  }), Object.defineProperty(e.prototype, "delimited", {
    get: function() {
      return this.resolvedType instanceof d && this._features.message_encoding === "DELIMITED";
    }
  }), Object.defineProperty(e.prototype, "packed", {
    get: function() {
      return this._features.repeated_field_encoding === "PACKED";
    }
  }), Object.defineProperty(e.prototype, "hasPresence", {
    get: function() {
      return this.repeated || this.map ? !1 : this.partOf || // oneofs
      this.declaringField || this.extensionField || // extensions
      this._features.field_presence !== "IMPLICIT";
    }
  }), e.prototype.setOption = function(i, t, l) {
    return a.prototype.setOption.call(this, i, t, l);
  }, e.prototype.toJSON = function(i) {
    var t = i ? !!i.keepComments : !1;
    return c.toObject([
      "edition",
      this._editionToJSON(),
      "rule",
      this.rule !== "optional" && this.rule || void 0,
      "type",
      this.type,
      "id",
      this.id,
      "extend",
      this.extend,
      "options",
      this.options,
      "comment",
      t ? this.comment : void 0
    ]);
  }, e.prototype.resolve = function() {
    if (this.resolved)
      return this;
    if ((this.typeDefault = h.defaults[this.type]) === void 0 ? (this.resolvedType = (this.declaringField ? this.declaringField.parent : this.parent).lookupTypeOrEnum(this.type), this.resolvedType instanceof d ? this.typeDefault = null : this.typeDefault = this.resolvedType.values[Object.keys(this.resolvedType.values)[0]]) : this.options && this.options.proto3_optional && (this.typeDefault = null), this.options && this.options.default != null && (this.typeDefault = this.options.default, this.resolvedType instanceof f && typeof this.typeDefault == "string" && (this.typeDefault = this.resolvedType.values[this.typeDefault])), this.options && (this.options.packed !== void 0 && this.resolvedType && !(this.resolvedType instanceof f) && delete this.options.packed, Object.keys(this.options).length || (this.options = void 0)), this.long)
      this.typeDefault = c.Long.fromNumber(this.typeDefault, this.type.charAt(0) === "u"), Object.freeze && Object.freeze(this.typeDefault);
    else if (this.bytes && typeof this.typeDefault == "string") {
      var i;
      c.base64.test(this.typeDefault) ? c.base64.decode(this.typeDefault, i = c.newBuffer(c.base64.length(this.typeDefault)), 0) : c.utf8.write(this.typeDefault, i = c.newBuffer(c.utf8.length(this.typeDefault)), 0), this.typeDefault = i;
    }
    return this.map ? this.defaultValue = c.emptyObject : this.repeated ? this.defaultValue = c.emptyArray : this.defaultValue = this.typeDefault, this.parent instanceof d && (this.parent.ctor.prototype[this.name] = this.defaultValue), a.prototype.resolve.call(this);
  }, e.prototype._inferLegacyProtoFeatures = function(i) {
    if (i !== "proto2" && i !== "proto3")
      return {};
    var t = {};
    if (this.rule === "required" && (t.field_presence = "LEGACY_REQUIRED"), this.parent && h.defaults[this.type] === void 0) {
      var l = this.parent.get(this.type.split(".").pop());
      l && l instanceof d && l.group && (t.message_encoding = "DELIMITED");
    }
    return this.getOption("packed") === !0 ? t.repeated_field_encoding = "PACKED" : this.getOption("packed") === !1 && (t.repeated_field_encoding = "EXPANDED"), t;
  }, e.prototype._resolveFeatures = function(i) {
    return a.prototype._resolveFeatures.call(this, this._edition || i);
  }, e.d = function(i, t, l, s) {
    return typeof t == "function" ? t = c.decorateType(t).name : t && typeof t == "object" && (t = c.decorateEnum(t).name), function(o, p) {
      c.decorateType(o.constructor).add(new e(p, i, t, l, { default: s }));
    };
  }, e._configure = function(i) {
    d = i;
  }, field;
}
var oneof, hasRequiredOneof;
function requireOneof() {
  if (hasRequiredOneof) return oneof;
  hasRequiredOneof = 1, oneof = c;
  var a = requireObject();
  ((c.prototype = Object.create(a.prototype)).constructor = c).className = "OneOf";
  var f = requireField(), h = requireUtil();
  function c(n, e, r, i) {
    if (Array.isArray(e) || (r = e, e = void 0), a.call(this, n, r), !(e === void 0 || Array.isArray(e)))
      throw TypeError("fieldNames must be an Array");
    this.oneof = e || [], this.fieldsArray = [], this.comment = i;
  }
  c.fromJSON = function(e, r) {
    return new c(e, r.oneof, r.options, r.comment);
  }, c.prototype.toJSON = function(e) {
    var r = e ? !!e.keepComments : !1;
    return h.toObject([
      "options",
      this.options,
      "oneof",
      this.oneof,
      "comment",
      r ? this.comment : void 0
    ]);
  };
  function d(n) {
    if (n.parent)
      for (var e = 0; e < n.fieldsArray.length; ++e)
        n.fieldsArray[e].parent || n.parent.add(n.fieldsArray[e]);
  }
  return c.prototype.add = function(e) {
    if (!(e instanceof f))
      throw TypeError("field must be a Field");
    return e.parent && e.parent !== this.parent && e.parent.remove(e), this.oneof.push(e.name), this.fieldsArray.push(e), e.partOf = this, d(this), this;
  }, c.prototype.remove = function(e) {
    if (!(e instanceof f))
      throw TypeError("field must be a Field");
    var r = this.fieldsArray.indexOf(e);
    if (r < 0)
      throw Error(e + " is not a member of " + this);
    return this.fieldsArray.splice(r, 1), r = this.oneof.indexOf(e.name), r > -1 && this.oneof.splice(r, 1), e.partOf = null, this;
  }, c.prototype.onAdd = function(e) {
    a.prototype.onAdd.call(this, e);
    for (var r = this, i = 0; i < this.oneof.length; ++i) {
      var t = e.get(this.oneof[i]);
      t && !t.partOf && (t.partOf = r, r.fieldsArray.push(t));
    }
    d(this);
  }, c.prototype.onRemove = function(e) {
    for (var r = 0, i; r < this.fieldsArray.length; ++r)
      (i = this.fieldsArray[r]).parent && i.parent.remove(i);
    a.prototype.onRemove.call(this, e);
  }, Object.defineProperty(c.prototype, "isProto3Optional", {
    get: function() {
      if (this.fieldsArray == null || this.fieldsArray.length !== 1)
        return !1;
      var n = this.fieldsArray[0];
      return n.options != null && n.options.proto3_optional === !0;
    }
  }), c.d = function() {
    for (var e = new Array(arguments.length), r = 0; r < arguments.length; )
      e[r] = arguments[r++];
    return function(t, l) {
      h.decorateType(t.constructor).add(new c(l, e)), Object.defineProperty(t, l, {
        get: h.oneOfGetter(e),
        set: h.oneOfSetter(e)
      });
    };
  }, oneof;
}
var object, hasRequiredObject;
function requireObject() {
  if (hasRequiredObject) return object;
  hasRequiredObject = 1, object = r, r.className = "ReflectionObject";
  const a = requireOneof();
  var f = requireUtil(), h, c = { enum_type: "OPEN", field_presence: "EXPLICIT", json_format: "ALLOW", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "PACKED", utf8_validation: "VERIFY", enforce_naming_style: "STYLE2024", default_symbol_visibility: "EXPORT_TOP_LEVEL" }, d = { enum_type: "OPEN", field_presence: "EXPLICIT", json_format: "ALLOW", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "PACKED", utf8_validation: "VERIFY", enforce_naming_style: "STYLE_LEGACY", default_symbol_visibility: "EXPORT_ALL" }, n = { enum_type: "CLOSED", field_presence: "EXPLICIT", json_format: "LEGACY_BEST_EFFORT", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "EXPANDED", utf8_validation: "NONE", enforce_naming_style: "STYLE_LEGACY", default_symbol_visibility: "EXPORT_ALL" }, e = { enum_type: "OPEN", field_presence: "IMPLICIT", json_format: "ALLOW", message_encoding: "LENGTH_PREFIXED", repeated_field_encoding: "PACKED", utf8_validation: "VERIFY", enforce_naming_style: "STYLE_LEGACY", default_symbol_visibility: "EXPORT_ALL" };
  function r(i, t) {
    if (!f.isString(i))
      throw TypeError("name must be a string");
    if (t && !f.isObject(t))
      throw TypeError("options must be an object");
    this.options = t, this.parsedOptions = null, this.name = i, this._edition = null, this._defaultEdition = "proto2", this._features = {}, this._featuresResolved = !1, this.parent = null, this.resolved = !1, this.comment = null, this.filename = null;
  }
  return Object.defineProperties(r.prototype, {
    /**
     * Reference to the root namespace.
     * @name ReflectionObject#root
     * @type {Root}
     * @readonly
     */
    root: {
      get: function() {
        for (var i = this; i.parent !== null; )
          i = i.parent;
        return i;
      }
    },
    /**
     * Full name including leading dot.
     * @name ReflectionObject#fullName
     * @type {string}
     * @readonly
     */
    fullName: {
      get: function() {
        for (var i = [this.name], t = this.parent; t; )
          i.unshift(t.name), t = t.parent;
        return i.join(".");
      }
    }
  }), r.prototype.toJSON = /* istanbul ignore next */
  function() {
    throw Error();
  }, r.prototype.onAdd = function(t) {
    this.parent && this.parent !== t && this.parent.remove(this), this.parent = t, this.resolved = !1;
    var l = t.root;
    l instanceof h && l._handleAdd(this);
  }, r.prototype.onRemove = function(t) {
    var l = t.root;
    l instanceof h && l._handleRemove(this), this.parent = null, this.resolved = !1;
  }, r.prototype.resolve = function() {
    return this.resolved ? this : (this.root instanceof h && (this.resolved = !0), this);
  }, r.prototype._resolveFeaturesRecursive = function(t) {
    return this._resolveFeatures(this._edition || t);
  }, r.prototype._resolveFeatures = function(t) {
    if (!this._featuresResolved) {
      var l = {};
      if (!t)
        throw new Error("Unknown edition for " + this.fullName);
      var s = Object.assign(
        this.options ? Object.assign({}, this.options.features) : {},
        this._inferLegacyProtoFeatures(t)
      );
      if (this._edition) {
        if (t === "proto2")
          l = Object.assign({}, n);
        else if (t === "proto3")
          l = Object.assign({}, e);
        else if (t === "2023")
          l = Object.assign({}, d);
        else if (t === "2024")
          l = Object.assign({}, c);
        else
          throw new Error("Unknown edition: " + t);
        this._features = Object.assign(l, s || {}), this._featuresResolved = !0;
        return;
      }
      if (this.partOf instanceof a) {
        var u = Object.assign({}, this.partOf._features);
        this._features = Object.assign(u, s || {});
      } else if (!this.declaringField) if (this.parent) {
        var o = Object.assign({}, this.parent._features);
        this._features = Object.assign(o, s || {});
      } else
        throw new Error("Unable to find a parent for " + this.fullName);
      this.extensionField && (this.extensionField._features = this._features), this._featuresResolved = !0;
    }
  }, r.prototype._inferLegacyProtoFeatures = function() {
    return {};
  }, r.prototype.getOption = function(t) {
    if (this.options)
      return this.options[t];
  }, r.prototype.setOption = function(t, l, s) {
    return this.options || (this.options = {}), /^features\\./.test(t) ? f.setProperty(this.options, t, l, s) : (!s || this.options[t] === void 0) && (this.getOption(t) !== l && (this.resolved = !1), this.options[t] = l), this;
  }, r.prototype.setParsedOption = function(t, l, s) {
    this.parsedOptions || (this.parsedOptions = []);
    var u = this.parsedOptions;
    if (s) {
      var o = u.find(function(E) {
        return Object.prototype.hasOwnProperty.call(E, t);
      });
      if (o) {
        var p = o[t];
        f.setProperty(p, s, l);
      } else
        o = {}, o[t] = f.setProperty({}, s, l), u.push(o);
    } else {
      var y = {};
      y[t] = l, u.push(y);
    }
    return this;
  }, r.prototype.setOptions = function(t, l) {
    if (t)
      for (var s = Object.keys(t), u = 0; u < s.length; ++u)
        this.setOption(s[u], t[s[u]], l);
    return this;
  }, r.prototype.toString = function() {
    var t = this.constructor.className, l = this.fullName;
    return l.length ? t + " " + l : t;
  }, r.prototype._editionToJSON = function() {
    if (!(!this._edition || this._edition === "proto3"))
      return this._edition;
  }, r._configure = function(i) {
    h = i;
  }, object;
}
var _enum, hasRequired_enum;
function require_enum() {
  if (hasRequired_enum) return _enum;
  hasRequired_enum = 1, _enum = c;
  var a = requireObject();
  ((c.prototype = Object.create(a.prototype)).constructor = c).className = "Enum";
  var f = requireNamespace(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (a.call(this, d, e), n && typeof n != "object")
      throw TypeError("values must be an object");
    if (this.valuesById = {}, this.values = Object.create(this.valuesById), this.comment = r, this.comments = i || {}, this.valuesOptions = t, this._valuesFeatures = {}, this.reserved = void 0, n)
      for (var l = Object.keys(n), s = 0; s < l.length; ++s)
        typeof n[l[s]] == "number" && (this.valuesById[this.values[l[s]] = n[l[s]]] = l[s]);
  }
  return c.prototype._resolveFeatures = function(n) {
    return n = this._edition || n, a.prototype._resolveFeatures.call(this, n), Object.keys(this.values).forEach((e) => {
      var r = Object.assign({}, this._features);
      this._valuesFeatures[e] = Object.assign(r, this.valuesOptions && this.valuesOptions[e] && this.valuesOptions[e].features);
    }), this;
  }, c.fromJSON = function(n, e) {
    var r = new c(n, e.values, e.options, e.comment, e.comments);
    return r.reserved = e.reserved, e.edition && (r._edition = e.edition), r._defaultEdition = "proto3", r;
  }, c.prototype.toJSON = function(n) {
    var e = n ? !!n.keepComments : !1;
    return h.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      this.options,
      "valuesOptions",
      this.valuesOptions,
      "values",
      this.values,
      "reserved",
      this.reserved && this.reserved.length ? this.reserved : void 0,
      "comment",
      e ? this.comment : void 0,
      "comments",
      e ? this.comments : void 0
    ]);
  }, c.prototype.add = function(n, e, r, i) {
    if (!h.isString(n))
      throw TypeError("name must be a string");
    if (!h.isInteger(e))
      throw TypeError("id must be an integer");
    if (this.values[n] !== void 0)
      throw Error("duplicate name '" + n + "' in " + this);
    if (this.isReservedId(e))
      throw Error("id " + e + " is reserved in " + this);
    if (this.isReservedName(n))
      throw Error("name '" + n + "' is reserved in " + this);
    if (this.valuesById[e] !== void 0) {
      if (!(this.options && this.options.allow_alias))
        throw Error("duplicate id " + e + " in " + this);
      this.values[n] = e;
    } else
      this.valuesById[this.values[n] = e] = n;
    return i && (this.valuesOptions === void 0 && (this.valuesOptions = {}), this.valuesOptions[n] = i || null), this.comments[n] = r || null, this;
  }, c.prototype.remove = function(n) {
    if (!h.isString(n))
      throw TypeError("name must be a string");
    var e = this.values[n];
    if (e == null)
      throw Error("name '" + n + "' does not exist in " + this);
    return delete this.valuesById[e], delete this.values[n], delete this.comments[n], this.valuesOptions && delete this.valuesOptions[n], this;
  }, c.prototype.isReservedId = function(n) {
    return f.isReservedId(this.reserved, n);
  }, c.prototype.isReservedName = function(n) {
    return f.isReservedName(this.reserved, n);
  }, _enum;
}
var encoder_1, hasRequiredEncoder;
function requireEncoder() {
  if (hasRequiredEncoder) return encoder_1;
  hasRequiredEncoder = 1, encoder_1 = d;
  var a = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n, e, r, i) {
    return e.delimited ? n("types[%i].encode(%s,w.uint32(%i)).uint32(%i)", r, i, (e.id << 3 | 3) >>> 0, (e.id << 3 | 4) >>> 0) : n("types[%i].encode(%s,w.uint32(%i).fork()).ldelim()", r, i, (e.id << 3 | 2) >>> 0);
  }
  function d(n) {
    for (var e = h.codegen(["m", "w"], n.name + "$encode")("if(!w)")("w=Writer.create()"), r, i, t = (
      /* initializes */
      n.fieldsArray.slice().sort(h.compareFieldsById)
    ), r = 0; r < t.length; ++r) {
      var l = t[r].resolve(), s = n._fieldsArray.indexOf(l), u = l.resolvedType instanceof a ? "int32" : l.type, o = f.basic[u];
      i = "m" + h.safeProp(l.name), l.map ? (e("if(%s!=null&&Object.hasOwnProperty.call(m,%j)){", i, l.name)("for(var ks=Object.keys(%s),i=0;i<ks.length;++i){", i)("w.uint32(%i).fork().uint32(%i).%s(ks[i])", (l.id << 3 | 2) >>> 0, 8 | f.mapKey[l.keyType], l.keyType), o === void 0 ? e("types[%i].encode(%s[ks[i]],w.uint32(18).fork()).ldelim().ldelim()", s, i) : e(".uint32(%i).%s(%s[ks[i]]).ldelim()", 16 | o, u, i), e("}")("}")) : l.repeated ? (e("if(%s!=null&&%s.length){", i, i), l.packed && f.packed[u] !== void 0 ? e("w.uint32(%i).fork()", (l.id << 3 | 2) >>> 0)("for(var i=0;i<%s.length;++i)", i)("w.%s(%s[i])", u, i)("w.ldelim()") : (e("for(var i=0;i<%s.length;++i)", i), o === void 0 ? c(e, l, s, i + "[i]") : e("w.uint32(%i).%s(%s[i])", (l.id << 3 | o) >>> 0, u, i)), e("}")) : (l.optional && e("if(%s!=null&&Object.hasOwnProperty.call(m,%j))", i, l.name), o === void 0 ? c(e, l, s, i) : e("w.uint32(%i).%s(%s)", (l.id << 3 | o) >>> 0, u, i));
    }
    return e("return w");
  }
  return encoder_1;
}
var hasRequiredIndexLight;
function requireIndexLight() {
  if (hasRequiredIndexLight) return indexLight.exports;
  hasRequiredIndexLight = 1;
  var a = indexLight.exports = requireIndexMinimal();
  a.build = "light";
  function f(c, d, n) {
    return typeof d == "function" ? (n = d, d = new a.Root()) : d || (d = new a.Root()), d.load(c, n);
  }
  a.load = f;
  function h(c, d) {
    return d || (d = new a.Root()), d.loadSync(c);
  }
  return a.loadSync = h, a.encoder = requireEncoder(), a.decoder = requireDecoder(), a.verifier = requireVerifier(), a.converter = requireConverter(), a.ReflectionObject = requireObject(), a.Namespace = requireNamespace(), a.Root = requireRoot(), a.Enum = require_enum(), a.Type = requireType(), a.Field = requireField(), a.OneOf = requireOneof(), a.MapField = requireMapfield(), a.Service = requireService(), a.Method = requireMethod(), a.Message = requireMessage(), a.wrappers = requireWrappers(), a.types = requireTypes(), a.util = requireUtil(), a.ReflectionObject._configure(a.Root), a.Namespace._configure(a.Type, a.Service, a.Enum), a.Root._configure(a.Type), a.Field._configure(a.Type), indexLight.exports;
}
var tokenize_1, hasRequiredTokenize;
function requireTokenize() {
  if (hasRequiredTokenize) return tokenize_1;
  hasRequiredTokenize = 1, tokenize_1 = l;
  var a = /[\\s{}=;:[\\],'"()<>]/g, f = /(?:"([^"\\\\]*(?:\\\\.[^"\\\\]*)*)")/g, h = /(?:'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)')/g, c = /^ *[*/]+ */, d = /^\\s*\\*?\\/*/, n = /\\n/g, e = /\\s/, r = /\\\\(.?)/g, i = {
    0: "\\0",
    r: "\\r",
    n: \`
\`,
    t: "	"
  };
  function t(s) {
    return s.replace(r, function(u, o) {
      switch (o) {
        case "\\\\":
        case "":
          return o;
        default:
          return i[o] || "";
      }
    });
  }
  l.unescape = t;
  function l(s, u) {
    s = s.toString();
    var o = 0, p = s.length, y = 1, E = 0, v = {}, m = [], _ = null;
    function A(k) {
      return Error("illegal " + k + " (line " + y + ")");
    }
    function I() {
      var k = _ === "'" ? h : f;
      k.lastIndex = o - 1;
      var P = k.exec(s);
      if (!P)
        throw A("string");
      return o = k.lastIndex, S(_), _ = null, t(P[1]);
    }
    function C(k) {
      return s.charAt(k);
    }
    function j(k, P, F) {
      var W = {
        type: s.charAt(k++),
        lineEmpty: !1,
        leading: F
      }, H;
      u ? H = 2 : H = 3;
      var D = k - H, $;
      do
        if (--D < 0 || ($ = s.charAt(D)) === \`
\`) {
          W.lineEmpty = !0;
          break;
        }
      while ($ === " " || $ === "	");
      for (var Y = s.substring(k, P).split(n), z = 0; z < Y.length; ++z)
        Y[z] = Y[z].replace(u ? d : c, "").trim();
      W.text = Y.join(\`
\`).trim(), v[y] = W, E = y;
    }
    function K(k) {
      var P = B(k), F = s.substring(k, P), W = /^\\s*\\/\\//.test(F);
      return W;
    }
    function B(k) {
      for (var P = k; P < p && C(P) !== \`
\`; )
        P++;
      return P;
    }
    function L() {
      if (m.length > 0)
        return m.shift();
      if (_)
        return I();
      var k, P, F, W, H, D = o === 0;
      do {
        if (o === p)
          return null;
        for (k = !1; e.test(F = C(o)); )
          if (F === \`
\` && (D = !0, ++y), ++o === p)
            return null;
        if (C(o) === "/") {
          if (++o === p)
            throw A("comment");
          if (C(o) === "/")
            if (u) {
              if (W = o, H = !1, K(o - 1)) {
                H = !0;
                do
                  if (o = B(o), o === p || (o++, !D))
                    break;
                while (K(o));
              } else
                o = Math.min(p, B(o) + 1);
              H && (j(W, o, D), D = !0), y++, k = !0;
            } else {
              for (H = C(W = o + 1) === "/"; C(++o) !== \`
\`; )
                if (o === p)
                  return null;
              ++o, H && (j(W, o - 1, D), D = !0), ++y, k = !0;
            }
          else if ((F = C(o)) === "*") {
            W = o + 1, H = u || C(W) === "*";
            do {
              if (F === \`
\` && ++y, ++o === p)
                throw A("comment");
              P = F, F = C(o);
            } while (P !== "*" || F !== "/");
            ++o, H && (j(W, o - 2, D), D = !0), k = !0;
          } else
            return "/";
        }
      } while (k);
      var $ = o;
      a.lastIndex = 0;
      var Y = a.test(C($++));
      if (!Y)
        for (; $ < p && !a.test(C($)); )
          ++$;
      var z = s.substring(o, o = $);
      return (z === '"' || z === "'") && (_ = z), z;
    }
    function S(k) {
      m.push(k);
    }
    function J() {
      if (!m.length) {
        var k = L();
        if (k === null)
          return null;
        S(k);
      }
      return m[0];
    }
    function M(k, P) {
      var F = J(), W = F === k;
      if (W)
        return L(), !0;
      if (!P)
        throw A("token '" + F + "', '" + k + "' expected");
      return !1;
    }
    function T(k) {
      var P = null, F;
      return k === void 0 ? (F = v[y - 1], delete v[y - 1], F && (u || F.type === "*" || F.lineEmpty) && (P = F.leading ? F.text : null)) : (E < k && J(), F = v[k], delete v[k], F && !F.lineEmpty && (u || F.type === "/") && (P = F.leading ? null : F.text)), P;
    }
    return Object.defineProperty({
      next: L,
      peek: J,
      push: S,
      skip: M,
      cmnt: T
    }, "line", {
      get: function() {
        return y;
      }
    });
  }
  return tokenize_1;
}
var parse_1, hasRequiredParse;
function requireParse() {
  if (hasRequiredParse) return parse_1;
  hasRequiredParse = 1, parse_1 = I, I.filename = null, I.defaults = { keepCase: !1 };
  var a = requireTokenize(), f = requireRoot(), h = requireType(), c = requireField(), d = requireMapfield(), n = requireOneof(), e = require_enum(), r = requireService(), i = requireMethod(), t = requireObject(), l = requireTypes(), s = requireUtil(), u = /^[1-9][0-9]*$/, o = /^-?[1-9][0-9]*$/, p = /^0[x][0-9a-fA-F]+$/, y = /^-?0[x][0-9a-fA-F]+$/, E = /^0[0-7]+$/, v = /^-?0[0-7]+$/, m = /^(?![eE])[0-9]*(?:\\.[0-9]*)?(?:[eE][+-]?[0-9]+)?$/, _ = /^[a-zA-Z_][a-zA-Z_0-9]*$/, A = /^(?:\\.?[a-zA-Z_][a-zA-Z_0-9]*)(?:\\.[a-zA-Z_][a-zA-Z_0-9]*)*$/;
  function I(C, j, K) {
    j instanceof f || (K = j, j = new f()), K || (K = I.defaults);
    var B = K.preferTrailingComment || !1, L = a(C, K.alternateCommentMode || !1), S = L.next, J = L.push, M = L.peek, T = L.skip, k = L.cmnt, P = !0, F, W, H, D = "proto2", $ = j, Y = [], z = {}, ae = K.keepCase ? function(R) {
      return R;
    } : s.camelCase;
    function he() {
      Y.forEach((R) => {
        R._edition = D, Object.keys(z).forEach((g) => {
          R.getOption(g) === void 0 && R.setOption(g, z[g], !0);
        });
      });
    }
    function N(R, g, O) {
      var b = I.filename;
      return O || (I.filename = null), Error("illegal " + (g || "token") + " '" + R + "' (" + (b ? b + ", " : "") + "line " + L.line + ")");
    }
    function ee() {
      var R = [], g;
      do {
        if ((g = S()) !== '"' && g !== "'")
          throw N(g);
        R.push(S()), T(g), g = M();
      } while (g === '"' || g === "'");
      return R.join("");
    }
    function ue(R) {
      var g = S();
      switch (g) {
        case "'":
        case '"':
          return J(g), ee();
        case "true":
        case "TRUE":
          return !0;
        case "false":
        case "FALSE":
          return !1;
      }
      try {
        return pe(
          g,
          /* insideTryCatch */
          !0
        );
      } catch {
        if (A.test(g))
          return g;
        throw N(g, "value");
      }
    }
    function re(R, g) {
      var O, b;
      do
        if (g && ((O = M()) === '"' || O === "'")) {
          var w = ee();
          if (R.push(w), D >= 2023)
            throw N(w, "id");
        } else
          try {
            R.push([b = te(S()), T("to", !0) ? te(S()) : b]);
          } catch (x) {
            if (g && A.test(O) && D >= 2023)
              R.push(O);
            else
              throw x;
          }
      while (T(",", !0));
      var q = { options: void 0 };
      q.setOption = function(x, G) {
        this.options === void 0 && (this.options = {}), this.options[x] = G;
      }, Z(
        q,
        function(G) {
          if (G === "option")
            Q(q, G), T(";");
          else
            throw N(G);
        },
        function() {
          se(q);
        }
      );
    }
    function pe(R, g) {
      var O = 1;
      switch (R.charAt(0) === "-" && (O = -1, R = R.substring(1)), R) {
        case "inf":
        case "INF":
        case "Inf":
          return O * (1 / 0);
        case "nan":
        case "NAN":
        case "Nan":
        case "NaN":
          return NaN;
        case "0":
          return 0;
      }
      if (u.test(R))
        return O * parseInt(R, 10);
      if (p.test(R))
        return O * parseInt(R, 16);
      if (E.test(R))
        return O * parseInt(R, 8);
      if (m.test(R))
        return O * parseFloat(R);
      throw N(R, "number", g);
    }
    function te(R, g) {
      switch (R) {
        case "max":
        case "MAX":
        case "Max":
          return 536870911;
        case "0":
          return 0;
      }
      if (!g && R.charAt(0) === "-")
        throw N(R, "id");
      if (o.test(R))
        return parseInt(R, 10);
      if (y.test(R))
        return parseInt(R, 16);
      if (v.test(R))
        return parseInt(R, 8);
      throw N(R, "id");
    }
    function ye() {
      if (F !== void 0)
        throw N("package");
      if (F = S(), !A.test(F))
        throw N(F, "name");
      $ = $.define(F), T(";");
    }
    function me() {
      var R = M(), g;
      switch (R) {
        case "option":
          if (D < "2024")
            throw N("option");
          S(), ee(), T(";");
          return;
        case "weak":
          g = H || (H = []), S();
          break;
        case "public":
          S();
        // eslint-disable-next-line no-fallthrough
        default:
          g = W || (W = []);
          break;
      }
      R = ee(), T(";"), g.push(R);
    }
    function ve() {
      if (T("="), D = ee(), D < 2023)
        throw N(D, "syntax");
      T(";");
    }
    function ge() {
      if (T("="), D = ee(), !["2023", "2024"].includes(D))
        throw N(D, "edition");
      T(";");
    }
    function ie(R, g) {
      switch (g) {
        case "option":
          return Q(R, g), T(";"), !0;
        case "message":
          return ne(R, g), !0;
        case "enum":
          return de(R, g), !0;
        case "export":
        case "local":
          return D < "2024" || (g = S(), g === "export" || g === "local") || g !== "message" && g !== "enum" ? !1 : ie(R, g);
        case "service":
          return be(R, g), !0;
        case "extend":
          return Se(R, g), !0;
      }
      return !1;
    }
    function Z(R, g, O) {
      var b = L.line;
      if (R && (typeof R.comment != "string" && (R.comment = k()), R.filename = I.filename), T("{", !0)) {
        for (var w; (w = S()) !== "}"; )
          g(w);
        T(";", !0);
      } else
        O && O(), T(";"), R && (typeof R.comment != "string" || B) && (R.comment = k(b) || R.comment);
    }
    function ne(R, g) {
      if (!_.test(g = S()))
        throw N(g, "type name");
      var O = new h(g);
      Z(O, function(w) {
        if (!ie(O, w))
          switch (w) {
            case "map":
              Ee(O);
              break;
            case "required":
              if (D !== "proto2")
                throw N(w);
            /* eslint-disable no-fallthrough */
            case "repeated":
              X(O, w);
              break;
            case "optional":
              if (D === "proto3")
                X(O, "proto3_optional");
              else {
                if (D !== "proto2")
                  throw N(w);
                X(O, "optional");
              }
              break;
            case "oneof":
              Re(O, w);
              break;
            case "extensions":
              re(O.extensions || (O.extensions = []));
              break;
            case "reserved":
              re(O.reserved || (O.reserved = []), !0);
              break;
            default:
              if (D === "proto2" || !A.test(w))
                throw N(w);
              J(w), X(O, "optional");
              break;
          }
      }), R.add(O), R === $ && Y.push(O);
    }
    function X(R, g, O) {
      var b = S();
      if (b === "group") {
        _e(R, g);
        return;
      }
      for (; b.endsWith(".") || M().startsWith("."); )
        b += S();
      if (!A.test(b))
        throw N(b, "type");
      var w = S();
      if (!_.test(w))
        throw N(w, "name");
      w = ae(w), T("=");
      var q = new c(w, te(S()), b, g, O);
      if (Z(q, function(U) {
        if (U === "option")
          Q(q, U), T(";");
        else
          throw N(U);
      }, function() {
        se(q);
      }), g === "proto3_optional") {
        var x = new n("_" + w);
        q.setOption("proto3_optional", !0), x.add(q), R.add(x);
      } else
        R.add(q);
      R === $ && Y.push(q);
    }
    function _e(R, g) {
      if (D >= 2023)
        throw N("group");
      var O = S();
      if (!_.test(O))
        throw N(O, "name");
      var b = s.lcFirst(O);
      O === b && (O = s.ucFirst(O)), T("=");
      var w = te(S()), q = new h(O);
      q.group = !0;
      var x = new c(b, w, O, g);
      x.filename = I.filename, Z(q, function(U) {
        switch (U) {
          case "option":
            Q(q, U), T(";");
            break;
          case "required":
          case "repeated":
            X(q, U);
            break;
          case "optional":
            D === "proto3" ? X(q, "proto3_optional") : X(q, "optional");
            break;
          case "message":
            ne(q, U);
            break;
          case "enum":
            de(q, U);
            break;
          case "reserved":
            re(q.reserved || (q.reserved = []), !0);
            break;
          case "export":
          case "local":
            if (D < "2024")
              throw N(U);
            switch (U = S(), U) {
              case "message":
                ne(q, U);
                break;
              case "enum":
                ne(q, U);
                break;
              default:
                throw N(U);
            }
            break;
          /* istanbul ignore next */
          default:
            throw N(U);
        }
      }), R.add(q).add(x);
    }
    function Ee(R) {
      T("<");
      var g = S();
      if (l.mapKey[g] === void 0)
        throw N(g, "type");
      T(",");
      var O = S();
      if (!A.test(O))
        throw N(O, "type");
      T(">");
      var b = S();
      if (!_.test(b))
        throw N(b, "name");
      T("=");
      var w = new d(ae(b), te(S()), g, O);
      Z(w, function(x) {
        if (x === "option")
          Q(w, x), T(";");
        else
          throw N(x);
      }, function() {
        se(w);
      }), R.add(w);
    }
    function Re(R, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var O = new n(ae(g));
      Z(O, function(w) {
        w === "option" ? (Q(O, w), T(";")) : (J(w), X(O, "optional"));
      }), R.add(O);
    }
    function de(R, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var O = new e(g);
      Z(O, function(w) {
        switch (w) {
          case "option":
            Q(O, w), T(";");
            break;
          case "reserved":
            re(O.reserved || (O.reserved = []), !0), O.reserved === void 0 && (O.reserved = []);
            break;
          default:
            Oe(O, w);
        }
      }), R.add(O), R === $ && Y.push(O);
    }
    function Oe(R, g) {
      if (!_.test(g))
        throw N(g, "name");
      T("=");
      var O = te(S(), !0), b = {
        options: void 0
      };
      b.getOption = function(w) {
        return this.options[w];
      }, b.setOption = function(w, q) {
        t.prototype.setOption.call(b, w, q);
      }, b.setParsedOption = function() {
      }, Z(b, function(q) {
        if (q === "option")
          Q(b, q), T(";");
        else
          throw N(q);
      }, function() {
        se(b);
      }), R.add(g, O, b.comment, b.parsedOptions || b.options);
    }
    function Q(R, g) {
      var O, b, w = !0;
      for (g === "option" && (g = S()); g !== "="; ) {
        if (g === "(") {
          var q = S();
          T(")"), g = "(" + q + ")";
        }
        if (w) {
          if (w = !1, g.includes(".") && !g.includes("(")) {
            var x = g.split(".");
            O = x[0] + ".", g = x[1];
            continue;
          }
          O = g;
        } else
          b = b ? b += g : g;
        g = S();
      }
      var G = b ? O.concat(b) : O, U = ce(R, G);
      b = b && b[0] === "." ? b.slice(1) : b, O = O && O[O.length - 1] === "." ? O.slice(0, -1) : O, Ae(R, O, U, b);
    }
    function ce(R, g) {
      if (T("{", !0)) {
        for (var O = {}; !T("}", !0); ) {
          if (!_.test(V = S()))
            throw N(V, "name");
          if (V === null)
            throw N(V, "end of input");
          var b, w = V;
          if (T(":", !0), M() === "{")
            b = ce(R, g + "." + V);
          else if (M() === "[") {
            b = [];
            var q;
            if (T("[", !0)) {
              do
                q = ue(), b.push(q);
              while (T(",", !0));
              T("]"), typeof q < "u" && fe(R, g + "." + V, q);
            }
          } else
            b = ue(), fe(R, g + "." + V, b);
          var x = O[w];
          x && (b = [].concat(x).concat(b)), O[w] = b, T(",", !0), T(";", !0);
        }
        return O;
      }
      var G = ue();
      return fe(R, g, G), G;
    }
    function fe(R, g, O) {
      if ($ === R && /^features\\./.test(g)) {
        z[g] = O;
        return;
      }
      R.setOption && R.setOption(g, O);
    }
    function Ae(R, g, O, b) {
      R.setParsedOption && R.setParsedOption(g, O, b);
    }
    function se(R) {
      if (T("[", !0)) {
        do
          Q(R, "option");
        while (T(",", !0));
        T("]");
      }
      return R;
    }
    function be(R, g) {
      if (!_.test(g = S()))
        throw N(g, "service name");
      var O = new r(g);
      Z(O, function(w) {
        if (!ie(O, w))
          if (w === "rpc")
            we(O, w);
          else
            throw N(w);
      }), R.add(O), R === $ && Y.push(O);
    }
    function we(R, g) {
      var O = k(), b = g;
      if (!_.test(g = S()))
        throw N(g, "name");
      var w = g, q, x, G, U;
      if (T("("), T("stream", !0) && (x = !0), !A.test(g = S()) || (q = g, T(")"), T("returns"), T("("), T("stream", !0) && (U = !0), !A.test(g = S())))
        throw N(g);
      G = g, T(")");
      var oe = new i(w, b, q, G, x, U);
      oe.comment = O, Z(oe, function(le) {
        if (le === "option")
          Q(oe, le), T(";");
        else
          throw N(le);
      }), R.add(oe);
    }
    function Se(R, g) {
      if (!A.test(g = S()))
        throw N(g, "reference");
      var O = g;
      Z(null, function(w) {
        switch (w) {
          case "required":
          case "repeated":
            X(R, w, O);
            break;
          case "optional":
            D === "proto3" ? X(R, "proto3_optional", O) : X(R, "optional", O);
            break;
          default:
            if (D === "proto2" || !A.test(w))
              throw N(w);
            J(w), X(R, "optional", O);
            break;
        }
      });
    }
    for (var V; (V = S()) !== null; )
      switch (V) {
        case "package":
          if (!P)
            throw N(V);
          ye();
          break;
        case "import":
          if (!P)
            throw N(V);
          me();
          break;
        case "syntax":
          if (!P)
            throw N(V);
          ve();
          break;
        case "edition":
          if (!P)
            throw N(V);
          ge();
          break;
        case "option":
          Q($, V), T(";", !0);
          break;
        default:
          if (ie($, V)) {
            P = !1;
            continue;
          }
          throw N(V);
      }
    return he(), I.filename = null, {
      package: F,
      imports: W,
      weakImports: H,
      root: j
    };
  }
  return parse_1;
}
var common_1, hasRequiredCommon;
function requireCommon() {
  if (hasRequiredCommon) return common_1;
  hasRequiredCommon = 1, common_1 = f;
  var a = /\\/|\\./;
  function f(c, d) {
    a.test(c) || (c = "google/protobuf/" + c + ".proto", d = { nested: { google: { nested: { protobuf: { nested: d } } } } }), f[c] = d;
  }
  f("any", {
    /**
     * Properties of a google.protobuf.Any message.
     * @interface IAny
     * @type {Object}
     * @property {string} [typeUrl]
     * @property {Uint8Array} [bytes]
     * @memberof common
     */
    Any: {
      fields: {
        type_url: {
          type: "string",
          id: 1
        },
        value: {
          type: "bytes",
          id: 2
        }
      }
    }
  });
  var h;
  return f("duration", {
    /**
     * Properties of a google.protobuf.Duration message.
     * @interface IDuration
     * @type {Object}
     * @property {number|Long} [seconds]
     * @property {number} [nanos]
     * @memberof common
     */
    Duration: h = {
      fields: {
        seconds: {
          type: "int64",
          id: 1
        },
        nanos: {
          type: "int32",
          id: 2
        }
      }
    }
  }), f("timestamp", {
    /**
     * Properties of a google.protobuf.Timestamp message.
     * @interface ITimestamp
     * @type {Object}
     * @property {number|Long} [seconds]
     * @property {number} [nanos]
     * @memberof common
     */
    Timestamp: h
  }), f("empty", {
    /**
     * Properties of a google.protobuf.Empty message.
     * @interface IEmpty
     * @memberof common
     */
    Empty: {
      fields: {}
    }
  }), f("struct", {
    /**
     * Properties of a google.protobuf.Struct message.
     * @interface IStruct
     * @type {Object}
     * @property {Object.<string,IValue>} [fields]
     * @memberof common
     */
    Struct: {
      fields: {
        fields: {
          keyType: "string",
          type: "Value",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.Value message.
     * @interface IValue
     * @type {Object}
     * @property {string} [kind]
     * @property {0} [nullValue]
     * @property {number} [numberValue]
     * @property {string} [stringValue]
     * @property {boolean} [boolValue]
     * @property {IStruct} [structValue]
     * @property {IListValue} [listValue]
     * @memberof common
     */
    Value: {
      oneofs: {
        kind: {
          oneof: [
            "nullValue",
            "numberValue",
            "stringValue",
            "boolValue",
            "structValue",
            "listValue"
          ]
        }
      },
      fields: {
        nullValue: {
          type: "NullValue",
          id: 1
        },
        numberValue: {
          type: "double",
          id: 2
        },
        stringValue: {
          type: "string",
          id: 3
        },
        boolValue: {
          type: "bool",
          id: 4
        },
        structValue: {
          type: "Struct",
          id: 5
        },
        listValue: {
          type: "ListValue",
          id: 6
        }
      }
    },
    NullValue: {
      values: {
        NULL_VALUE: 0
      }
    },
    /**
     * Properties of a google.protobuf.ListValue message.
     * @interface IListValue
     * @type {Object}
     * @property {Array.<IValue>} [values]
     * @memberof common
     */
    ListValue: {
      fields: {
        values: {
          rule: "repeated",
          type: "Value",
          id: 1
        }
      }
    }
  }), f("wrappers", {
    /**
     * Properties of a google.protobuf.DoubleValue message.
     * @interface IDoubleValue
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    DoubleValue: {
      fields: {
        value: {
          type: "double",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.FloatValue message.
     * @interface IFloatValue
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    FloatValue: {
      fields: {
        value: {
          type: "float",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.Int64Value message.
     * @interface IInt64Value
     * @type {Object}
     * @property {number|Long} [value]
     * @memberof common
     */
    Int64Value: {
      fields: {
        value: {
          type: "int64",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.UInt64Value message.
     * @interface IUInt64Value
     * @type {Object}
     * @property {number|Long} [value]
     * @memberof common
     */
    UInt64Value: {
      fields: {
        value: {
          type: "uint64",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.Int32Value message.
     * @interface IInt32Value
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    Int32Value: {
      fields: {
        value: {
          type: "int32",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.UInt32Value message.
     * @interface IUInt32Value
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    UInt32Value: {
      fields: {
        value: {
          type: "uint32",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.BoolValue message.
     * @interface IBoolValue
     * @type {Object}
     * @property {boolean} [value]
     * @memberof common
     */
    BoolValue: {
      fields: {
        value: {
          type: "bool",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.StringValue message.
     * @interface IStringValue
     * @type {Object}
     * @property {string} [value]
     * @memberof common
     */
    StringValue: {
      fields: {
        value: {
          type: "string",
          id: 1
        }
      }
    },
    /**
     * Properties of a google.protobuf.BytesValue message.
     * @interface IBytesValue
     * @type {Object}
     * @property {Uint8Array} [value]
     * @memberof common
     */
    BytesValue: {
      fields: {
        value: {
          type: "bytes",
          id: 1
        }
      }
    }
  }), f("field_mask", {
    /**
     * Properties of a google.protobuf.FieldMask message.
     * @interface IDoubleValue
     * @type {Object}
     * @property {number} [value]
     * @memberof common
     */
    FieldMask: {
      fields: {
        paths: {
          rule: "repeated",
          type: "string",
          id: 1
        }
      }
    }
  }), f.get = function(d) {
    return f[d] || null;
  }, common_1;
}
var hasRequiredSrc;
function requireSrc() {
  if (hasRequiredSrc) return src.exports;
  hasRequiredSrc = 1;
  var a = src.exports = requireIndexLight();
  return a.build = "full", a.tokenize = requireTokenize(), a.parse = requireParse(), a.common = requireCommon(), a.Root._configure(a.Type, a.parse, a.common), src.exports;
}
var protobufjs, hasRequiredProtobufjs;
function requireProtobufjs() {
  return hasRequiredProtobufjs || (hasRequiredProtobufjs = 1, protobufjs = requireSrc()), protobufjs;
}
var protobufjsExports = requireProtobufjs(), StateStreamErrorCode = /* @__PURE__ */ ((a) => (a.CONNECTION_FAILED = "CONNECTION_FAILED", a.RECONNECT_FAILED = "RECONNECT_FAILED", a.CONNECTION_LOST = "CONNECTION_LOST", a.CONNECTION_TIMEOUT = "CONNECTION_TIMEOUT", a.AUTH_FAILED = "AUTH_FAILED", a.AUTH_REFRESH_FAILED = "AUTH_REFRESH_FAILED", a.DEVICE_ERROR = "DEVICE_ERROR", a.DECODE_ERROR = "DECODE_ERROR", a.FRAME_PROCESS_ERROR = "FRAME_PROCESS_ERROR", a.STREAM_ALREADY_STARTED = "STREAM_ALREADY_STARTED", a.WORKER_INIT_FAILED = "WORKER_INIT_FAILED", a.UNKNOWN_ERROR = "UNKNOWN_ERROR", a))(StateStreamErrorCode || {}), ConnectionStatus = /* @__PURE__ */ ((a) => (a.DISCONNECTED = "DISCONNECTED", a.CONNECTING = "CONNECTING", a.CONNECTED = "CONNECTED", a.RECONNECTING = "RECONNECTING", a))(ConnectionStatus || {}), AuthStatus = /* @__PURE__ */ ((a) => (a.UNAUTHENTICATED = "UNAUTHENTICATED", a.AUTHENTICATING = "AUTHENTICATING", a.AUTHENTICATED = "AUTHENTICATED", a.REAUTHENTICATING = "REAUTHENTICATING", a.FAILED = "FAILED", a))(AuthStatus || {});
const DEFAULT_MAX_RECONNECT_ATTEMPTS = 5, DEFAULT_MAX_AUTH_ATTEMPTS = 5, DEFAULT_DELAY_RECONNECT = 500, nested = { BSB_State: { nested: { StateUpdate: { oneofs: { state: { oneof: ["deviceName", "power", "brightness", "audioVolume", "wifi", "updateState", "updateCheck", "timezone", "matter", "frame", "input", "timer", "ble", "autoUpdateState"] } }, fields: { deviceName: { type: "BSB_State.DeviceName", id: 1 }, power: { type: "BSB_State.Power", id: 2 }, brightness: { type: "BSB_State.Brightness", id: 3 }, audioVolume: { type: "BSB_State.AudioVolume", id: 4 }, wifi: { type: "BSB_State.Wifi", id: 5 }, updateState: { type: "BSB_Update.UpdateState", id: 6 }, updateCheck: { type: "BSB_Update.CheckState", id: 7 }, timezone: { type: "BSB_State.Timezone", id: 8 }, matter: { type: "BSB_State.Matter", id: 9 }, frame: { type: "BSB_Frame.Frame", id: 10 }, input: { type: "BSB_Input.InputEvent", id: 11 }, timer: { type: "BSB_Timer.Timer", id: 12 }, ble: { type: "BSB_State.Ble.Ble", id: 13 }, autoUpdateState: { type: "BSB_Update.AutoUpdateState", id: 14 } } }, State: { oneofs: { _error: { oneof: ["error"] } }, fields: { timestamp: { type: "fixed64", id: 1 }, updates: { rule: "repeated", type: "StateUpdate", id: 2 }, error: { type: "BSB_Error.Error", id: 3, options: { proto3_optional: !0 } } } }, DeviceName: { fields: { name: { type: "string", id: 1 } } }, BrightnessAutomatic: { fields: {} }, BrightnessManual: { fields: { brightness: { type: "uint32", id: 1 } } }, Brightness: { oneofs: { setting: { oneof: ["automatic", "manual"] } }, fields: { automatic: { type: "BrightnessAutomatic", id: 1 }, manual: { type: "BrightnessManual", id: 2 }, actualBrightness: { type: "uint32", id: 3 } } }, BatteryStatus: { values: { DISCHARGING: 0, CHARGING: 1, CHARGED: 2 } }, UnknownPowerState: { fields: {} }, PowerState: { fields: { batteryStatus: { type: "BatteryStatus", id: 1 }, batteryChargePercent: { type: "uint32", id: 2 }, batteryVoltageMv: { type: "uint32", id: 3 }, batteryCurrentMa: { type: "sint32", id: 4 }, usbVoltageMv: { type: "uint32", id: 5 } } }, Power: { oneofs: { state: { oneof: ["unknown", "known"] } }, fields: { unknown: { type: "UnknownPowerState", id: 1 }, known: { type: "PowerState", id: 2 } } }, AudioVolume: { fields: { volume: { type: "uint32", id: 1 } } }, WifiConnectionStatus: { values: { CONNECTED: 0, CONNECTING: 1, DISCONNECTING: 2, RECONNECTING: 3 } }, WifiSecurity: { values: { UNKNOWN: 0, OPEN: 1, WPA: 2, WPA2: 3, WEP: 4, WPA_WPA2: 5, WPA3: 6, WPA2_WPA3: 7 } }, IpConfigurationMethod: { values: { DHCP: 0, STATIC: 1 } }, IpProtocol: { values: { IPV4: 0, IPV6: 1 } }, WifiStateUnknown: { fields: {} }, WifiStateDisconnected: { fields: {} }, WifiStateConnected: { fields: { status: { type: "WifiConnectionStatus", id: 1 }, ssid: { type: "string", id: 2 }, bssid: { type: "string", id: 3 }, channel: { type: "uint32", id: 4 }, rssi: { type: "sint32", id: 5 }, security: { type: "WifiSecurity", id: 6 } } }, IpAddress: { fields: { protocol: { type: "IpProtocol", id: 1 }, method: { type: "IpConfigurationMethod", id: 2 }, address: { type: "string", id: 3 }, gateway: { type: "string", id: 4 }, netmask: { type: "string", id: 5 } } }, Wifi: { oneofs: { wifiState: { oneof: ["unknown", "disconnected", "connected"] } }, fields: { unknown: { type: "WifiStateUnknown", id: 1 }, disconnected: { type: "WifiStateDisconnected", id: 2 }, connected: { type: "WifiStateConnected", id: 3 }, ipAddresses: { rule: "repeated", type: "IpAddress", id: 4 } } }, Timezone: { fields: { name: { type: "string", id: 1 }, offset: { type: "sint32", id: 2 }, abbr: { type: "string", id: 3 } } }, MatterCommissioningStatus: { values: { NEVER_STARTED: 0, STARTED: 1, COMPLETED_SUCCESSFULLY: 2, FAILED: 3 } }, MatterCommissioningState: { fields: { status: { type: "MatterCommissioningStatus", id: 1 }, timestamp: { type: "fixed64", id: 2 } } }, Matter: { fields: { fabricCount: { type: "uint32", id: 1 }, state: { type: "MatterCommissioningState", id: 2 } } }, Ble: { nested: { ServiceStatus: { values: { RESET: 0, INITIALIZATION: 1, READY: 2, ADVERTISING: 3, CONNECTABLE: 4, CONNECTED: 5, ERROR: 6 } }, Ble: { oneofs: { _remoteAddress: { oneof: ["remoteAddress"] } }, fields: { status: { type: "ServiceStatus", id: 1 }, remoteAddress: { type: "string", id: 2, options: { proto3_optional: !0 } } } } } } } }, BSB_Update: { nested: { UpdateEvent: { values: { SESSION_START: 0, SESSION_STOP: 1, ACTION_BEGIN: 2, ACTION_DONE: 3, DETAIL_CHANGE: 4, ACTION_PROGRESS: 5, EVENT_NONE: 6 } }, UpdateAction: { values: { DOWNLOAD: 0, SHA_VERIFICATION: 1, UNPACK: 2, INSTALLATION_PREPARE: 3, INSTALLATION_APPLY: 4, ACTION_NONE: 5 } }, UpdateStatus: { values: { OK: 0, BATTERY_LOW: 1, BUSY: 2, DOWNLOAD_FAILURE: 3, DOWNLOAD_ABORT: 4, SHA_MISMATCH: 5, UNPACK_CREATE_STAGING_DIRECTORY_FAILURE: 6, UNPACK_ARCHIVE_OPEN_FAILURE: 7, UNPACK_ARCHIVE_UNPACK_FAILURE: 8, INSTALLATION_PREPARE_MANIFEST_NOT_FOUND: 9, INSTALLATION_PREPARE_MANIFEST_INVALID: 10, INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE: 11, INSTALLATION_PREPARE_POINTER_SETUP_FAILURE: 12, UNKNOWN_FAILURE: 13 } }, CheckError: { values: { NOT_AVAILABLE: 0, FAILURE: 1, IDLE: 2 } }, UpdateAvailable: { fields: { version: { type: "string", id: 1 } } }, UpdateUnavailable: { fields: { reason: { type: "CheckError", id: 1 } } }, UpdateState: { fields: { event: { type: "UpdateEvent", id: 1 }, action: { type: "UpdateAction", id: 2 }, status: { type: "UpdateStatus", id: 3 } } }, CheckState: { oneofs: { status: { oneof: ["available", "unavailable"] } }, fields: { available: { type: "UpdateAvailable", id: 1 }, unavailable: { type: "UpdateUnavailable", id: 2 } } }, AutoUpdateInterval: { fields: { start: { type: "uint32", id: 1 }, end: { type: "uint32", id: 2 } } }, AutoUpdateState: { fields: { enabled: { type: "bool", id: 1 }, interval: { type: "AutoUpdateInterval", id: 2 } } } } }, BSB_Frame: { nested: { Encoding: { values: { PLAIN: 0, RUN_LENGTH: 1, DEFLATE: 2, DEFLATE_RUN_LENGTH: 3 } }, PixelFormat: { values: { RGB888: 0, L8: 1, L4: 2 } }, Screen: { values: { FRONT: 0, BACK: 1 } }, Frame: { fields: { screen: { type: "Screen", id: 1 }, width: { type: "uint32", id: 2 }, height: { type: "uint32", id: 3 }, encoding: { type: "Encoding", id: 4 }, pixelFormat: { type: "PixelFormat", id: 5 }, data: { type: "bytes", id: 6 } } } } }, BSB_Timer: { nested: { Timer: { fields: { json: { type: "BSB_Util.Json", id: 1 } } } } }, BSB_Util: { nested: { Compression: { values: { PLAIN: 0, GZIP: 1 } }, Json: { fields: { compression: { type: "Compression", id: 1 }, data: { type: "bytes", id: 2 } } } } }, BSB_Input: { nested: { Button: { values: { OK: 0, BACK: 1, START: 2 } }, ButtonAction: { values: { PRESS: 0, RELEASE: 1 } }, SwitchPosition: { values: { BUSY: 0, CUSTOM: 1, OFF: 2, APPS: 3, SETTINGS: 4 } }, ButtonEvent: { fields: { button: { type: "Button", id: 1 }, action: { type: "ButtonAction", id: 2 } } }, SwitchEvent: { fields: { position: { type: "SwitchPosition", id: 1 } } }, EncoderEvent: { fields: { delta: { type: "sint32", id: 1 } } }, InputEvent: { oneofs: { event: { oneof: ["buttonEvent", "switchEvent", "encoderEvent"] } }, fields: { buttonEvent: { type: "ButtonEvent", id: 1 }, switchEvent: { type: "SwitchEvent", id: 2 }, encoderEvent: { type: "EncoderEvent", id: 3 } } } } }, BSB_Error: { nested: { Cause: { values: { RESOURCE_LIMIT: 0 } }, Severity: { values: { FATAL: 0, ERROR: 1, WARNING: 2 } }, Error: { fields: { cause: { type: "Cause", id: 1 }, severity: { type: "Severity", id: 2 } } } } } };
var bundle = {
  nested
};
function decompressRLE(a, f) {
  const h = [];
  for (let c = 0; c < a.length; ) {
    const d = a[c++];
    if (d === void 0) break;
    const n = d & 127;
    if (!n)
      continue;
    if (d & 128) {
      const r = n * f, i = a.subarray(c, c + r);
      for (let t = 0; t < i.length; t++)
        h.push(i[t]);
      c += r;
      continue;
    }
    const e = a.subarray(c, c + f);
    c += f;
    for (let r = 0; r < n; r++)
      for (let i = 0; i < f; i++)
        h.push(e[i]);
  }
  return new Uint8Array(h);
}
async function decompressDeflate(a) {
  if (typeof DecompressionStream > "u")
    throw new Error("DecompressionStream is not supported in this environment.");
  try {
    const f = new DecompressionStream("deflate"), h = f.writable.getWriter();
    h.write(a), h.close();
    const d = await new Response(f.readable).arrayBuffer();
    return new Uint8Array(d);
  } catch (f) {
    throw new Error(\`Deflate decompression failed: \${f instanceof Error ? f.message : String(f)}\`);
  }
}
function convertL4toRGBA(a, f, h) {
  const c = new Uint8ClampedArray(f * h * 4);
  let d = 0;
  for (let n = 0; n < a.length; n++) {
    const e = a[n], r = (e & 15) * 17, i = (e >> 4 & 15) * 17, t = [r, i];
    for (const l of t)
      if (d < f * h) {
        const s = d * 4;
        c[s] = l, c[s + 1] = l, c[s + 2] = l, c[s + 3] = 255, d++;
      }
  }
  return c;
}
function convertL8toRGBA(a, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = Math.min(a.length, f * h);
  for (let n = 0; n < d; n++) {
    const e = a[n], r = n * 4;
    c[r] = e, c[r + 1] = e, c[r + 2] = e, c[r + 3] = 255;
  }
  return c;
}
function convertRGB888toRGBA(a, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = f * h;
  for (let n = 0; n < d; n++) {
    const e = n * 3, r = n * 4;
    e + 2 < a.length && (c[r] = a[e + 2], c[r + 1] = a[e + 1], c[r + 2] = a[e], c[r + 3] = 255);
  }
  return c;
}
async function processFrame(a) {
  if (!a.data || !a.width || !a.height)
    return null;
  let f = a.data;
  const h = a.pixelFormat === 0 ? 3 : 1;
  switch (a.encoding) {
    case 1:
      f = decompressRLE(f, h);
      break;
    case 2:
      f = await decompressDeflate(f);
      break;
    case 3:
      f = await decompressDeflate(f), f = decompressRLE(f, h);
      break;
  }
  switch (a.pixelFormat) {
    case 2:
      return convertL4toRGBA(f, a.width, a.height);
    case 1:
      return convertL8toRGBA(f, a.width, a.height);
    case 0:
      return convertRGB888toRGBA(f, a.width, a.height);
    default:
      return new Uint8ClampedArray(a.width * a.height * 4);
  }
}
const root = protobufjsExports.Root.fromJSON(bundle), StateType = root.lookupType("BSB_State.State"), AUTH_CODE = 3e3;
let maxAuthAttempts = DEFAULT_MAX_AUTH_ATTEMPTS, maxReconnectAttempts = DEFAULT_MAX_RECONNECT_ATTEMPTS, reconnectDelay = DEFAULT_DELAY_RECONNECT, socket = null, isBinaryMode = !0, currentMode = "local", currentToken, currentAddr = "", retryCount = 0, authRetryCount = 0, isAuthReported = !1;
const activePorts = /* @__PURE__ */ new Set(), subscriptions = /* @__PURE__ */ new Map();
let processingQueue = Promise.resolve();
function broadcast(a) {
  for (const f of activePorts)
    f.postMessage(a);
}
function sendAuth() {
  currentMode === "remote" && currentToken && (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && (broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATING }), socket.send(JSON.stringify({ token: currentToken })));
}
function sendSubscriptions() {
  (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && subscriptions.size > 0 && socket.send(
    JSON.stringify({
      subscribe: Array.from(subscriptions.keys())
    })
  );
}
function stopSocket() {
  socket && (socket.onopen = null, socket.onmessage = null, socket.onerror = null, socket.onclose = null, socket.close(), socket = null);
}
function stopAndCleanup() {
  stopSocket(), subscriptions.clear(), activePorts.clear(), retryCount = 0, authRetryCount = 0, isAuthReported = !1;
}
function connect(a, f, h = !0, c = "local") {
  stopSocket(), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.CONNECTING }), currentAddr = a, isBinaryMode = h, currentMode = c, currentToken = f, isAuthReported = !1;
  const d = new URL(a);
  socket = new WebSocket(d.toString()), socket.binaryType = "arraybuffer", socket.onopen = () => {
    broadcast({ type: "CONNECTED" }), currentMode === "local" && (socket == null || socket.send(JSON.stringify({ enable: !0 }))), sendAuth(), retryCount = 0, authRetryCount = 0, console.log("[Worker] Connection stable. All retry counters reset."), currentMode === "remote" && subscriptions.size > 0 && sendSubscriptions(), currentMode === "local" && broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED });
  }, socket.onmessage = (n) => {
    processingQueue = processingQueue.then(async () => {
      try {
        let e = null, r = "", i;
        if (isBinaryMode)
          n.data instanceof ArrayBuffer && (e = new Uint8Array(n.data), r = e);
        else {
          const t = JSON.parse(n.data);
          i = t.bar_id || t.barId, r = n.data, t.state && (typeof t.state == "string" ? e = Uint8Array.from(atob(t.state), (l) => l.charCodeAt(0)) : e = new Uint8Array(t.state)), currentMode === "remote" && !isAuthReported && (isAuthReported = !0, broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED }));
        }
        if (r && broadcast({ type: "RAW_DATA", data: r }), e) {
          const t = StateType.decode(e), l = StateType.toObject(t, {
            longs: Number,
            bytes: Uint8Array,
            enums: Number,
            defaults: !0
          });
          if (l.error) {
            const { cause: s, severity: u } = l.error;
            if (s != null && u != null) {
              const o = root.lookupEnum("BSB_Error.Cause"), p = root.lookupEnum("BSB_Error.Severity"), y = o.valuesById[s] || "UNKNOWN", E = p.valuesById[u] || "UNKNOWN";
              if (broadcast({
                type: "ERROR",
                code: StateStreamErrorCode.DEVICE_ERROR,
                message: \`Server reported \${E}: \${y}\`,
                data: l.error
              }), u === p.values.FATAL) {
                stopAndCleanup();
                return;
              }
              if (u === p.values.ERROR)
                return;
            } else
              broadcast({
                type: "ERROR",
                code: StateStreamErrorCode.DEVICE_ERROR,
                message: "Server reported an unspecified application error",
                data: l.error
              });
          }
          if (l.updates)
            for (const s of l.updates) {
              const u = s.frame;
              if (u && u.data)
                try {
                  const o = await processFrame(u);
                  o && (u.data = o);
                } catch (o) {
                  broadcast({
                    type: "ERROR",
                    code: StateStreamErrorCode.FRAME_PROCESS_ERROR,
                    message: o instanceof Error ? o.message : String(o),
                    data: u.data
                  });
                }
            }
          broadcast(currentMode === "remote" && i ? {
            type: "DATA",
            data: { bar_id: i, state: l }
          } : { type: "DATA", data: l });
        }
      } catch (e) {
        broadcast({
          type: "ERROR",
          code: StateStreamErrorCode.DECODE_ERROR,
          message: \`Decode error: \${String(e)}\`,
          data: n.data
        });
      }
    }).catch(console.error);
  }, socket.onerror = () => {
    broadcast({ type: "ERROR", code: StateStreamErrorCode.CONNECTION_FAILED, message: "WebSocket connection error" });
  }, socket.onclose = (n) => {
    if (console.log("[Worker] Socket closed:", n), !socket || activePorts.size === 0) {
      console.log("[Worker] Connection closed or no active ports. No retries.");
      return;
    }
    if (n.code === AUTH_CODE && currentMode === "remote") {
      authRetryCount < maxAuthAttempts ? (authRetryCount++, console.warn(\`[Worker] Auth failed (3000). Requesting new token... (Attempt \${authRetryCount}/\${maxAuthAttempts})\`), broadcast({ type: "TOKEN_EXPIRED" }), broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.REAUTHENTICATING, authAttempts: authRetryCount })) : (broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.FAILED }), broadcast({
        type: "ERROR",
        code: StateStreamErrorCode.AUTH_FAILED,
        message: \`Maximum authentication attempts (\${maxAuthAttempts}) reached. Please log in again.\`
      }));
      return;
    }
    if (n.code !== 1e3) {
      if (retryCount < maxReconnectAttempts) {
        retryCount++;
        let e = Math.min(1e3 * retryCount, 5e3);
        reconnectDelay && (e = reconnectDelay), console.log(\`[Worker] Reconnecting (network code: \${n.code}) in \${e}ms... (Attempt \${retryCount}/\${maxReconnectAttempts})\`), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.RECONNECTING, connectionAttempts: retryCount }), setTimeout(() => {
          activePorts.size > 0 && socket && connect(currentAddr, currentToken, isBinaryMode, currentMode);
        }, e);
      } else
        broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), broadcast({
          type: "ERROR",
          code: StateStreamErrorCode.RECONNECT_FAILED,
          message: \`Maximum reconnection attempts (\${maxReconnectAttempts}) reached. Connection lost.\`
        });
      return;
    }
    broadcast({ type: "DISCONNECTED" }), broadcast({
      type: "ERROR",
      code: StateStreamErrorCode.CONNECTION_LOST,
      message: \`Stream closed with unexpected code: \${n.code}. Stopping stream.\`
    });
  };
}
function handleCommand(a, f) {
  switch (a.type) {
    case "START":
      maxAuthAttempts = a.maxAuthAttempts ?? DEFAULT_MAX_AUTH_ATTEMPTS, maxReconnectAttempts = a.maxReconnectAttempts ?? DEFAULT_MAX_RECONNECT_ATTEMPTS, reconnectDelay = a.reconnectDelay ?? DEFAULT_DELAY_RECONNECT, activePorts.add(f), socket && socket.readyState === WebSocket.OPEN && currentAddr === a.addr ? (f.postMessage({ type: "CONNECTED" }), f.postMessage({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED })) : connect(a.addr, a.token, a.isBinary, a.mode);
      break;
    case "STOP":
      activePorts.delete(f);
      for (const [e, r] of subscriptions.entries())
        r.delete(f), r.size === 0 && (subscriptions.delete(e), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [e] })));
      f.postMessage({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), activePorts.size === 0 && stopAndCleanup();
      break;
    case "UPDATE_TOKEN":
      const h = currentToken;
      if (currentToken = a.token, currentMode === "remote") {
        const e = socket && socket.readyState === WebSocket.OPEN;
        if (e && h === a.token)
          return;
        e ? sendAuth() : h !== a.token && currentAddr && activePorts.size > 0 && connect(currentAddr, currentToken, isBinaryMode, currentMode);
      }
      break;
    case "SUBSCRIBE":
      let c = subscriptions.get(a.guid);
      c || (c = /* @__PURE__ */ new Set(), subscriptions.set(a.guid, c));
      const d = c.size === 0;
      c.add(f), d && (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ subscribe: [a.guid] }));
      break;
    case "UNSUBSCRIBE":
      const n = subscriptions.get(a.guid);
      n && (n.delete(f), n.size === 0 && (subscriptions.delete(a.guid), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [a.guid] }))));
      break;
  }
}
if ("SharedWorkerGlobalScope" in self) {
  const a = self;
  a.onconnect = (f) => {
    const h = f.ports[0];
    h && (h.onmessage = (c) => handleCommand(c.data, h), h.start());
  };
} else {
  const a = self;
  a.onmessage = (f) => {
    handleCommand(f.data, a);
  };
}
`;function Hr(t){return new SharedWorker("data:text/javascript;charset=utf-8,"+encodeURIComponent(zr),{type:"module",name:t?.name})}const Ye=class de{constructor(e,r){S(this,"addr"),S(this,"token"),S(this,"isBinary"),S(this,"connectTimeout"),S(this,"dataTimeout"),S(this,"maxReconnectAttempts"),S(this,"maxAuthAttempts"),S(this,"reconnectDelay"),S(this,"worker",null),S(this,"connectionTimer",null),S(this,"dataTimer",null),S(this,"_status"),S(this,"dataCallback"),S(this,"rawDataCallback"),S(this,"errorCallback"),S(this,"statusCallback"),this.addr=e.addr||"",this.token=e.token,this.isBinary=e.isBinary??!0,this.connectTimeout=r?.timeout??5e3,this.dataTimeout=r?.dataTimeout??15e3,this.maxReconnectAttempts=r?.maxReconnectAttempts??Wr,this.maxAuthAttempts=Gr,this.reconnectDelay=r?.reconnectDelay??$r,this._status={main:{status:D.IDLE},connection:{status:Y.DISCONNECTED},auth:{status:re.UNAUTHENTICATED},data:{status:j.NONE},worker:{status:ae.OFF}}}get status(){return this._status}resolveProtocol(e){let r=e.trim();if(r.startsWith("https://"))return r.replace("https://","wss://");if(r.startsWith("http://"))return r.replace("http://","ws://");if(r.startsWith("wss://")||r.startsWith("ws://"))return r;let i="ws:";return typeof window<"u"&&window.location.protocol==="https:"&&(i="wss:"),`${i}//${r}`}start({dataCallback:e,rawDataCallback:r,errorCallback:i,statusCallback:n}){if(this._status.main.status===D.STARTING||this._status.main.status===D.RUNNING){const o=new te(L.STREAM_ALREADY_STARTED,"StateStream is already running. Call stop() before starting again.");if(n&&n({...this._status,main:{...this._status.main,lastError:o}}),i)i(o);else throw o;return}this.dataCallback=e,this.rawDataCallback=r,this.errorCallback=i,this.statusCallback=n,this.updateStatusComponent("main",{status:D.STARTING,lastError:void 0});try{this.ensureWorker(),this.sendCommand({type:"START",addr:this.normalizeUrl(this.addr),token:this.token,isBinary:this.isBinary,mode:this.streamMode,maxReconnectAttempts:this.maxReconnectAttempts,maxAuthAttempts:this.maxAuthAttempts,reconnectDelay:this.reconnectDelay}),this.clearConnectionTimer(),this.connectionTimer=setTimeout(()=>{const o=new te(L.CONNECTION_TIMEOUT,`Connection timed out after ${this.connectTimeout}ms`);this.mapErrorToStatus(o),this.errorCallback&&this.errorCallback(o),this.stop()},this.connectTimeout)}catch(o){const s=o instanceof te?o:new te(L.UNKNOWN_ERROR,String(o));this.errorCallback&&this.errorCallback(s)}}stop(){this.clearConnectionTimer(),this.clearDataTimer(),!(this._status.main.status===D.IDLE||this._status.main.status===D.STOPPED)&&(this.updateStatusComponent("main",{status:D.STOPPED}),this.updateStatusComponent("connection",{status:Y.DISCONNECTED}),this.sendCommand({type:"STOP"}),this.clearCallbacks())}destroy(){this.stop(),this.worker&&(this.worker.terminate?this.worker.terminate():"close"in this.worker.port&&this.worker.port.close(),this.worker=null)}clearCallbacks(){this.dataCallback=void 0,this.rawDataCallback=void 0,this.errorCallback=void 0,this.statusCallback=void 0}sendToken(e){this.token=e,this.sendCommand({type:"UPDATE_TOKEN",token:e})}sendCommand(e){this.worker&&this.worker.port.postMessage(e)}ensureWorker(){if(this.worker||typeof window>"u")return;const e=btoa(this.addr);try{if(this.updateStatusComponent("worker",{status:ae.INITIALIZING,lastError:void 0}),window.SharedWorker){const r=new Hr({name:e});this.worker={port:r.port},r.port.onmessage=i=>{this.handleWorkerMessage(i.data)},r.port.start()}else{const r=new Vr;this.worker={port:r,terminate:()=>r.terminate()},r.onmessage=i=>{this.handleWorkerMessage(i.data)}}this.updateStatusComponent("worker",{status:ae.READY})}catch(r){const i=new te(L.WORKER_INIT_FAILED,`Failed to initialize worker: ${String(r)}`);throw this.updateStatusComponent("worker",{status:ae.ERROR,lastError:i}),this.updateStatusComponent("main",{status:D.FAILED,lastError:i}),i}}handleWorkerMessage(e){switch(e.type){case"DATA":this.resetDataTimer(),this.dataCallback&&this.dataCallback(this.normalizeState(e.data));break;case"RAW_DATA":this.resetDataTimer(),this.rawDataCallback&&this.rawDataCallback(e.data);break;case"CONNECTED":this.clearConnectionTimer(),this.updateStatusComponent("connection",{status:Y.CONNECTED}),this.streamMode==="local"&&this.updateStatusComponent("main",{status:D.RUNNING});break;case"STATUS_UPDATE":if(e.connection){const r={status:e.connection,attempts:e.connection===Y.RECONNECTING?e.connectionAttempts:void 0};this.updateStatusComponent("connection",r)}if(e.auth){const r={status:e.auth,attempts:e.auth===re.REAUTHENTICATING?e.authAttempts:void 0};this.updateStatusComponent("auth",r),e.auth===re.AUTHENTICATED&&this.updateStatusComponent("main",{status:D.RUNNING})}break;case"ERROR":{this.clearConnectionTimer();const r=new te(e.code,e.message,e.data);this.mapErrorToStatus(r),this.errorCallback&&this.errorCallback(r);break}case"TOKEN_EXPIRED":this.updateStatusComponent("auth",{status:re.AUTHENTICATING}),this.handleTokenExpiredInternal();break;case"DISCONNECTED":this.updateStatusComponent("connection",{status:Y.DISCONNECTED});break}}mapErrorToStatus(e){const r=e.code;(r===L.CONNECTION_FAILED||r===L.CONNECTION_LOST||r===L.RECONNECT_FAILED||r===L.CONNECTION_TIMEOUT)&&(this.updateStatusComponent("connection",{status:Y.DISCONNECTED,lastError:e}),this.updateStatusComponent("main",{status:D.FAILED,lastError:e})),(r===L.AUTH_FAILED||r===L.AUTH_REFRESH_FAILED)&&(this.updateStatusComponent("auth",{status:re.FAILED,lastError:e}),this.updateStatusComponent("main",{status:D.FAILED,lastError:e})),(r===L.DEVICE_ERROR||r===L.DECODE_ERROR)&&this.updateStatusComponent("main",{lastError:e})}updateStatusComponent(e,r){const i={...this._status[e],...r};Object.keys(i).forEach(n=>{i[n]===void 0&&delete i[n]}),this._status[e]=i,this.statusCallback&&this.statusCallback({...this._status})}clearConnectionTimer(){this.connectionTimer&&(clearTimeout(this.connectionTimer),this.connectionTimer=null)}resetDataTimer(){this.clearDataTimer(),this._status.data.status!==j.ACTIVE?this.updateStatusComponent("data",{status:j.ACTIVE,lastActivity:Date.now()}):this._status.data.lastActivity=Date.now(),this.dataTimer=setTimeout(()=>{this.updateStatusComponent("data",{status:j.STALE})},this.dataTimeout)}clearDataTimer(){this.dataTimer&&(clearTimeout(this.dataTimer),this.dataTimer=null)}normalizeState(e){let r,i;"bar_id"in e&&"state"in e?(r=e.state,i=e.bar_id):r=e;let n=r.updates;return n&&(n=n.map(o=>{const s=Object.keys(o).find(a=>o[a]!=null);return{...o,state:s}})),{...r,updates:n,bar_id:i}}async handleTokenExpiredInternal(){const e=this.addr;let r=de.tokenRefreshPromises.get(e);if(!r&&this.onTokenExpired&&(r=(async()=>{try{const i=this.onTokenExpired();return i instanceof Promise?await i:""}finally{de.tokenRefreshPromises.delete(e)}})(),de.tokenRefreshPromises.set(e,r)),r){const i=await r;i&&this.sendToken(i)}}};S(Ye,"tokenRefreshPromises",new Map);let Jr=Ye;class Kr extends Jr{constructor(e={},r){let i=e.addr;i||(typeof window<"u"?i=window.location.origin:i="10.0.4.20"),super({isBinary:!0,...e,addr:i},r),S(this,"streamMode","local")}normalizeUrl(e){const r=this.resolveProtocol(e),i=new URL(r);return(i.pathname==="/"||!i.pathname)&&(i.pathname="/api/status/ws"),this.token&&i.searchParams.set("x-api-token",this.token),i.toString()}}var k;(t=>{(e=>{e[e.DISCHARGING=0]="DISCHARGING",e[e.CHARGING=1]="CHARGING",e[e.CHARGED=2]="CHARGED"})(t.BatteryStatus||(t.BatteryStatus={})),(e=>{e[e.CONNECTED=0]="CONNECTED",e[e.CONNECTING=1]="CONNECTING",e[e.DISCONNECTING=2]="DISCONNECTING",e[e.RECONNECTING=3]="RECONNECTING"})(t.WifiConnectionStatus||(t.WifiConnectionStatus={})),(e=>{e[e.UNKNOWN=0]="UNKNOWN",e[e.OPEN=1]="OPEN",e[e.WPA=2]="WPA",e[e.WPA2=3]="WPA2",e[e.WEP=4]="WEP",e[e.WPA_WPA2=5]="WPA_WPA2",e[e.WPA3=6]="WPA3",e[e.WPA2_WPA3=7]="WPA2_WPA3"})(t.WifiSecurity||(t.WifiSecurity={})),(e=>{e[e.DHCP=0]="DHCP",e[e.STATIC=1]="STATIC"})(t.IpConfigurationMethod||(t.IpConfigurationMethod={})),(e=>{e[e.IPV4=0]="IPV4",e[e.IPV6=1]="IPV6"})(t.IpProtocol||(t.IpProtocol={})),(e=>{e[e.NEVER_STARTED=0]="NEVER_STARTED",e[e.STARTED=1]="STARTED",e[e.COMPLETED_SUCCESSFULLY=2]="COMPLETED_SUCCESSFULLY",e[e.FAILED=3]="FAILED"})(t.MatterCommissioningStatus||(t.MatterCommissioningStatus={})),(e=>{(r=>{r[r.RESET=0]="RESET",r[r.INITIALIZATION=1]="INITIALIZATION",r[r.READY=2]="READY",r[r.ADVERTISING=3]="ADVERTISING",r[r.CONNECTABLE=4]="CONNECTABLE",r[r.CONNECTED=5]="CONNECTED",r[r.ERROR=6]="ERROR"})(e.ServiceStatus||(e.ServiceStatus={}))})(t.Ble||(t.Ble={}))})(k||(k={}));var he;(t=>{(e=>{e[e.SESSION_START=0]="SESSION_START",e[e.SESSION_STOP=1]="SESSION_STOP",e[e.ACTION_BEGIN=2]="ACTION_BEGIN",e[e.ACTION_DONE=3]="ACTION_DONE",e[e.DETAIL_CHANGE=4]="DETAIL_CHANGE",e[e.ACTION_PROGRESS=5]="ACTION_PROGRESS",e[e.EVENT_NONE=6]="EVENT_NONE"})(t.UpdateEvent||(t.UpdateEvent={})),(e=>{e[e.DOWNLOAD=0]="DOWNLOAD",e[e.SHA_VERIFICATION=1]="SHA_VERIFICATION",e[e.UNPACK=2]="UNPACK",e[e.INSTALLATION_PREPARE=3]="INSTALLATION_PREPARE",e[e.INSTALLATION_APPLY=4]="INSTALLATION_APPLY",e[e.ACTION_NONE=5]="ACTION_NONE"})(t.UpdateAction||(t.UpdateAction={})),(e=>{e[e.OK=0]="OK",e[e.BATTERY_LOW=1]="BATTERY_LOW",e[e.BUSY=2]="BUSY",e[e.DOWNLOAD_FAILURE=3]="DOWNLOAD_FAILURE",e[e.DOWNLOAD_ABORT=4]="DOWNLOAD_ABORT",e[e.SHA_MISMATCH=5]="SHA_MISMATCH",e[e.UNPACK_CREATE_STAGING_DIRECTORY_FAILURE=6]="UNPACK_CREATE_STAGING_DIRECTORY_FAILURE",e[e.UNPACK_ARCHIVE_OPEN_FAILURE=7]="UNPACK_ARCHIVE_OPEN_FAILURE",e[e.UNPACK_ARCHIVE_UNPACK_FAILURE=8]="UNPACK_ARCHIVE_UNPACK_FAILURE",e[e.INSTALLATION_PREPARE_MANIFEST_NOT_FOUND=9]="INSTALLATION_PREPARE_MANIFEST_NOT_FOUND",e[e.INSTALLATION_PREPARE_MANIFEST_INVALID=10]="INSTALLATION_PREPARE_MANIFEST_INVALID",e[e.INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE=11]="INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE",e[e.INSTALLATION_PREPARE_POINTER_SETUP_FAILURE=12]="INSTALLATION_PREPARE_POINTER_SETUP_FAILURE",e[e.UNKNOWN_FAILURE=13]="UNKNOWN_FAILURE"})(t.UpdateStatus||(t.UpdateStatus={})),(e=>{e[e.NOT_AVAILABLE=0]="NOT_AVAILABLE",e[e.FAILURE=1]="FAILURE",e[e.IDLE=2]="IDLE"})(t.CheckError||(t.CheckError={}))})(he||(he={}));var ke;(t=>{(e=>{e[e.PLAIN=0]="PLAIN",e[e.RUN_LENGTH=1]="RUN_LENGTH",e[e.DEFLATE=2]="DEFLATE",e[e.DEFLATE_RUN_LENGTH=3]="DEFLATE_RUN_LENGTH"})(t.Encoding||(t.Encoding={})),(e=>{e[e.RGB888=0]="RGB888",e[e.L8=1]="L8",e[e.L4=2]="L4"})(t.PixelFormat||(t.PixelFormat={})),(e=>{e[e.FRONT=0]="FRONT",e[e.BACK=1]="BACK"})(t.Screen||(t.Screen={}))})(ke||(ke={}));var Ie;(t=>{(e=>{e[e.PLAIN=0]="PLAIN",e[e.GZIP=1]="GZIP"})(t.Compression||(t.Compression={}))})(Ie||(Ie={}));var qe;(t=>{(e=>{e[e.OK=0]="OK",e[e.BACK=1]="BACK",e[e.START=2]="START"})(t.Button||(t.Button={})),(e=>{e[e.PRESS=0]="PRESS",e[e.RELEASE=1]="RELEASE"})(t.ButtonAction||(t.ButtonAction={})),(e=>{e[e.BUSY=0]="BUSY",e[e.CUSTOM=1]="CUSTOM",e[e.OFF=2]="OFF",e[e.APPS=3]="APPS",e[e.SETTINGS=4]="SETTINGS"})(t.SwitchPosition||(t.SwitchPosition={}))})(qe||(qe={}));var Pe;(t=>{(e=>{e[e.RESOURCE_LIMIT=0]="RESOURCE_LIMIT"})(t.Cause||(t.Cause={})),(e=>{e[e.FATAL=0]="FATAL",e[e.ERROR=1]="ERROR",e[e.WARNING=2]="WARNING"})(t.Severity||(t.Severity={}))})(Pe||(Pe={}));const Xe=class pe{constructor(){if(S(this,"gl",null),S(this,"program",null),S(this,"texture",null),S(this,"vs",`#version 300 es
    in vec2 position;
    out vec2 v_uv;
    void main() {
      v_uv = position * 0.5 + 0.5;
      v_uv.y = 1.0 - v_uv.y;
      gl_Position = vec4(position, 0, 1);
    }
  `),S(this,"fs",`#version 300 es
    precision highp float;
    in vec2 v_uv;
    out vec4 outColor;
    uniform sampler2D u_texture;
    uniform vec2 u_dataRes;
    uniform vec2 u_canvasRes;
    uniform float u_pixelSize;
    uniform float u_radius;
    uniform float u_darkThreshold;
    void main() {
      vec2 gridPos = v_uv * u_dataRes;
      vec2 localUv = fract(gridPos);
      vec2 cellCoords = (floor(gridPos) + 0.5) / u_dataRes;
      vec4 color = texture(u_texture, cellCoords);
      if (dot(color.rgb, vec3(1.0)) < u_darkThreshold) discard;
      float halfSize = u_pixelSize * 0.5;
      float visualRadius = sqrt(clamp(u_radius, 0.0, 1.0));
      float r = visualRadius * halfSize;
      vec2 q = abs(localUv - 0.5) - (halfSize - r);
      float dist = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
      float unitsPerPixel = (u_dataRes.y / u_canvasRes.y);
      float delta = unitsPerPixel * 1.5; 
      float effectRes = (1.0 - u_pixelSize) + u_radius;
      float effectStrength = smoothstep(0.0, 0.02, effectRes);
      float edgeOffset = mix(0.0, -delta, effectStrength);
      float mask = 1.0 - smoothstep(edgeOffset, edgeOffset + delta * 2.0, dist);
      float vignette = smoothstep(0.7, 0.3, length(localUv - 0.5));
      vec3 finalColor = color.rgb * (1.0 - (0.15 * effectStrength * (1.0 - vignette)));
      if (mask < 0.001) discard;
      outColor = vec4(finalColor, color.a * mask);
    }
  `),pe.instance)return pe.instance;pe.instance=this}init(){if(this.gl||typeof window>"u")return;const e=document.createElement("canvas").getContext("webgl2",{antialias:!0,alpha:!0,preserveDrawingBuffer:!0});if(!e)throw new Error("WebGL 2.0 not supported");this.gl=e,this.program=this.createProgram(this.vs,this.fs);const r=e.createBuffer();e.bindBuffer(e.ARRAY_BUFFER,r),e.bufferData(e.ARRAY_BUFFER,new Float32Array([-1,-1,1,-1,-1,1,-1,1,1,-1,1,1]),e.STATIC_DRAW);const i=e.getAttribLocation(this.program,"position");e.enableVertexAttribArray(i),e.vertexAttribPointer(i,2,e.FLOAT,!1,0,0),this.texture=e.createTexture(),e.bindTexture(e.TEXTURE_2D,this.texture),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_MIN_FILTER,e.NEAREST),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_MAG_FILTER,e.NEAREST),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_WRAP_S,e.CLAMP_TO_EDGE),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_WRAP_T,e.CLAMP_TO_EDGE)}render(e,r,i,n){const{pixelSize:o=.85,radius:s=.5,darkThreshold:a=.04}=n,l=this.gl;l.viewport(0,0,l.canvas.width,l.canvas.height),l.useProgram(this.program),l.bindTexture(l.TEXTURE_2D,this.texture),l.texImage2D(l.TEXTURE_2D,0,l.RGBA8,r,i,0,l.RGBA,l.UNSIGNED_BYTE,e),l.uniform2f(l.getUniformLocation(this.program,"u_dataRes"),r,i),l.uniform2f(l.getUniformLocation(this.program,"u_canvasRes"),l.canvas.width,l.canvas.height),l.uniform1f(l.getUniformLocation(this.program,"u_pixelSize"),o),l.uniform1f(l.getUniformLocation(this.program,"u_radius"),s),l.uniform1f(l.getUniformLocation(this.program,"u_darkThreshold"),a),l.clearColor(0,0,0,0),l.clear(l.COLOR_BUFFER_BIT),l.enable(l.BLEND),l.blendFunc(l.SRC_ALPHA,l.ONE_MINUS_SRC_ALPHA),l.drawArrays(l.TRIANGLES,0,6)}renderFrame(e,r,i,n,o={}){if(this.init(),typeof window>"u"||!this.gl)return;(this.gl.canvas.width!==e.width||this.gl.canvas.height!==e.height)&&(this.gl.canvas.width=e.width,this.gl.canvas.height=e.height),this.render(r,i,n,o);const s=e.getContext("2d");s&&(s.clearRect(0,0,e.width,e.height),s.drawImage(this.gl.canvas,0,0))}createProgram(e,r){const i=this.gl,n=(s,a)=>{const l=i.createShader(s);if(i.shaderSource(l,a),i.compileShader(l),!i.getShaderParameter(l,i.COMPILE_STATUS))throw new Error(i.getShaderInfoLog(l)||"Shader Error");return l},o=i.createProgram();return i.attachShader(o,n(i.VERTEX_SHADER,e)),i.attachShader(o,n(i.FRAGMENT_SHADER,r)),i.linkProgram(o),o}};S(Xe,"instance",null);let Yr=Xe;const Ci=new Yr,ie=M("apiStore",()=>{const t=ye().public.barUrl||window.location.origin,e=R(null);async function r(i,n){const o={...n?.headers||{}};return e.value&&(o["X-API-Token"]=e.value),$fetch(i,{baseURL:t,...n,headers:o})}return{apiKey:e,apiRequest:r}},{persist:{key:"apiStore",storage:Ge.sessionStorage()}}),ue=M("config",()=>{const t=R(!1),e=R(!1),r=R([{name:"httpPollingInterval",label:"HTTP polling interval (ms). Sound and brightness are polled 6x less often",type:"number",value:5e3,default:5e3},{name:"httpRequestTimeout",label:"Default HTTP request timeout (ms). Long requests like file upload don't have a timeout (browser default applies)",type:"number",value:3e3,default:3e3},{name:"notificationDuration",label:"Notification duration (ms). Error notifications never close automatically",type:"number",value:1e4,default:1e4},{name:"refreshDeviceDataAbortIfStreamActive",label:"Abort HTTP polling attempt if stream is active",type:"boolean",value:!0,default:!0},{name:"screenStreamCanvasBaseResolutionWidth",label:"Canvas resolution width. Gets multiplied by screen DPR for final resolution. Height is calculated based on screen aspect ratio",type:"number",value:720,default:720},{name:"sliderDebounceDelay",label:"Debounce delay for slider inputs (ms)",type:"number",value:250,default:250},{name:"stateStreamDataTimeout",label:"State stream data timeout (ms)",type:"number",value:1500,default:1500},{name:"stateStreamLogFrames",label:"Log state stream frames to console (debug level)",type:"boolean",value:!1,default:!1},{name:"stateStreamLogHeartbeats",label:"Log state stream heartbeats to console (debug level)",type:"boolean",value:!1,default:!1},{name:"stateStreamLogStatusUpdates",label:"Log state stream status updates to console (debug level). This includes all status messages (main/connection/auth/data/worker)",type:"boolean",value:!0,default:!0},{name:"stateStreamLogUpdates",label:"Log state stream updates to console (debug level). Disabling removes frames, heartbeats and state updates from console.debug",type:"boolean",value:!0,default:!0},{name:"stateStreamMaxReconnectAttempts",label:"State stream max reconnect attempts",type:"number",value:5,default:5},{name:"stateStreamReconnectDelay",label:"State stream reconnect delay (ms)",type:"number",value:250,default:250},{name:"stateStreamTimeout",label:"State stream socket timeout (ms)",type:"number",value:5e3,default:5e3},{name:"wifiAbortSimultaneousRequests",label:"Abort heavy WiFi HTTP request (list/connect/disconnect) if the same request is in progress",type:"boolean",value:!0,default:!0}]);function i(n){const o=r.value.find(s=>s.name===n);return o?o.value:void 0}return{showConfigUI:t,pinPopover:e,items:r,get:i}},{persist:{key:"configStore",storage:Ge.localStorage(),omit:["items"]}});async function O(t,e,r,i){const n=X(),o=ue();if(t?.status===403){await nt("/login");return}console.error(e,t),r&&(console.debug("[HTTP error handler]: checking device connection..."),await n.checkConnection()),n.isConnected&&F.add({id:"device-status-error",title:e,description:Zr(t),icon:"i-bi-alert",color:"error",duration:typeof i=="number"?i:Number(o.get("notificationDuration"))})}const Xr="Unknown error. Check your connection and try again.";function Zr(t){if(t?.data?.error)return t.data.error;if(String(t).length){const e=String(t);if(e.includes("Error:")){const r=e.indexOf("Error:");return e.slice(r+6).trim()}return e}return Xr}const Ee=globalThis.setInterval;var H=(t=>(t[t.IDLE=0]="IDLE",t[t.LOADING=1]="LOADING",t[t.UPDATING=2]="UPDATING",t[t.ERROR=3]="ERROR",t[t.SUCCESS=4]="SUCCESS",t))(H||{});const Ae=M("firmware",()=>{const t=X(),e=ue(),r=1800*1e3,i=R({status:null,availableVersion:null,isAllowed:null,isChecking:!1,isManualCheck:!1,backgroundCheckInterval:null,modals:{changelog:!1,batteryLow:!1,updating:!1,success:!1},changelog:null,isChangelogLoading:!1,stage:0,progress:0,progressPollingInterval:null,error:{stage:0,message:null}}),n=R({is_enabled:!1,interval_start:"02:00",interval_end:"05:00"});async function o(){return ie().apiRequest("/api/update/autoupdate").then(h=>(n.value={is_enabled:h.is_enabled,interval_start:h.interval_start,interval_end:h.interval_end},n.value)).catch(async h=>{await O(h,"Couldn't fetch auto-update self-check settings")})}async function s(h){return ie().apiRequest("/api/update/autoupdate",{method:"POST",body:h}).then(()=>{n.value=h}).catch(async E=>{await O(E,"Couldn't update auto-update self-check settings")})}function a(){i.value.status=null,i.value.availableVersion=null,i.value.isAllowed=null,i.value.changelog=null,i.value.progress=0,i.value.progressPollingInterval&&(clearInterval(i.value.progressPollingInterval),i.value.progressPollingInterval=null),i.value.error.stage=0,i.value.error.message=null}async function l(h=0){return t.busyBar.UpdateStatusGet().then(async E=>{if(!E.check?.status||!E.check.event)throw new Error("Invalid update status response: missing check info");if(!E.install)throw new Error("Invalid update status response: missing install info");if(E.check.event==="stop"&&E.check.status==="failure"){console.warn("Auto-update check failed",E),i.value.isChecking=!1,i.value.isManualCheck&&(i.value.isManualCheck=!1,F.add({title:"Update check failed",description:"Check your internet connection and try again.",icon:"i-bi-alert",color:"error",duration:0,close:!0,closeIcon:"i-bi-cross"}));return}if(E.check.event!=="stop"&&E.check.status==="none"){if(E.check.event==="none")return console.debug("Empty auto update status, requesting update check"),f();if(console.debug("Auto-update check still in progress, fetching status again"),await new Promise(q=>{setTimeout(q,3e3)}),h>=10)throw new Error("Auto-update check is taking too long, please try again later");return l(h?h+1:1)}if(i.value.isChecking=!1,i.value.status=E.check.status||null,i.value.availableVersion=E.check.available_version||null,i.value.isAllowed=!!E.install.is_allowed,i.value.isManualCheck&&E.check.status==="not_available"&&(i.value.isManualCheck=!1,F.add({title:"Your firmware version is up to date",icon:"i-bi-checkmark-circle-fill",color:"success",duration:Number(e.get("notificationDuration"))})),console.debug("Auto-update check completed",E),i.value.availableVersion)return c(i.value.availableVersion)}).catch(async E=>{i.value.isChecking=!1,await O(E,"Couldn't check for updates")})}async function f(){if(i.value.isChecking){console.debug("Already checking for updates, ignoring request");return}return i.value.isChecking=!0,t.busyBar.UpdateCheck().then(async()=>(console.debug("Auto-update check requested"),await new Promise(h=>setTimeout(h,1e3)),l())).catch(async h=>{if(h.status===409){console.debug("Auto-update check already in progress");return}i.value.isChecking=!1,await O(h,"Update check request failed")})}function d(){i.value.backgroundCheckInterval=Ee(()=>{console.debug(`Performing background auto-update check (${new Date().toISOString()})`),f()},r)}function u(){i.value.backgroundCheckInterval&&(clearInterval(i.value.backgroundCheckInterval),i.value.backgroundCheckInterval=null)}async function c(h){i.value.isChangelogLoading=!0,await t.busyBar.UpdateChangelogGet({version:h}).then(E=>{i.value.changelog=E.changelog||null}).catch(async E=>(await O(E,"Couldn't fetch update changelog"),null)).finally(()=>{i.value.isChangelogLoading=!1})}async function p(){if(!i.value.availableVersion){console.error("No available version to install");return}return console.debug("Requesting auto-update installation"),t.busyBar.UpdateInstall({version:i.value.availableVersion,timeout:0})}async function y(){await t.busyBar.UpdateAbort().then(()=>{console.debug("Auto-update download abort requested"),i.value.modals.updating=!1,i.value.stage=0,i.value.progress=0}).catch(async h=>{await O(h,"Couldn't abort update download")})}async function g(){console.debug("Starting auto-update process"),i.value.progress=0,i.value.error.stage=0,i.value.error.message=null,i.value.stage=1,await p().catch(async h=>{await O(h,"Update failed")}),i.value.progressPollingInterval=Ee(async()=>{await t.busyBar.UpdateStatusGet().then(h=>{if(!h.install)throw new Error("Invalid update status response: missing install info");if(h.install.event==="session_stop"){if(i.value.progressPollingInterval&&(clearInterval(i.value.progressPollingInterval),i.value.progressPollingInterval=null),h.install.status==="ok")return;if(h.install.status==="busy"){console.warn("Received session_stop event with status busy. Is this a firmware bug?");return}else if(h.install.status==="download_abort"){console.warn("Update download was aborted"),i.value.modals.updating=!1,i.value.stage=0,i.value.progress=0,F.add({title:"Update aborted",description:"The update download has been aborted.",icon:"i-bi-alert",color:"error",duration:0,close:!0,closeIcon:"i-bi-cross"});return}i.value.error.stage=i.value.stage,i.value.error.message=`Update failed with status: ${h.install.status}`,i.value.stage=3;return}if(h.install.status!=="ok"&&h.install.status!=="busy"){console.error("Update failed with status:",h),i.value.error.stage=i.value.stage,i.value.error.message=`Update failed: ${h.install.status}`,i.value.stage=3,clearInterval(i.value.progressPollingInterval);return}if(h.install.action==="download"){let E=Number(h.install.download?.total_bytes);isNaN(E)&&(console.warn("Received invalid total_bytes value in update status, defaulting to 0",h.install.download?.total_bytes),E=0);let q=Number(h.install.download?.received_bytes);isNaN(q)&&(console.warn("Received invalid received_bytes value in update status, defaulting to 0",h.install.download?.received_bytes),q=0),i.value.stage=1,E>0&&(i.value.progress=Math.round(q/E*100))}else h.install.action!=="none"&&(i.value.stage=2,i.value.progress=0,clearInterval(i.value.progressPollingInterval))}).catch(async h=>{await O(h,"Couldn't fetch update status"),i.value.stage=3,clearInterval(i.value.progressPollingInterval)})},1e3),i.value.modals.changelog=!1,i.value.modals.updating=!0}const b=R({firmwareBundleName:"firmware",firmwareFile:null,showFileUploadModal:!1,stage:0,progress:0,error:null});async function T(){const h=new XMLHttpRequest;h.open("POST",`${ye().public.barUrl||window.location.origin}/api/update`),h.setRequestHeader("Content-Type","application/octet-stream"),ie().apiKey&&h.setRequestHeader("X-API-Token",ie().apiKey),h.upload.onprogress=E=>{E.lengthComputable&&(b.value.progress=Math.round(E.loaded/E.total*100),i.value.progress=b.value.progress,b.value.progress===100&&console.debug("Firmware file upload completed, waiting for device to unpack"))},h.onload=()=>{h.status>=200&&h.status<400?(console.debug("Upload and unpacking complete, waiting for device to reboot"),b.value.stage=2,F.add({title:"Update initiated",description:"The device will reboot to apply the update. Pay attention to the front screen.",icon:"i-bi-checkmark-circle-fill",color:"success",duration:Number(e.get("notificationDuration"))})):(console.error("Upload failed:",h.status,h.responseText),b.value.stage=3,F.add({title:"Update failed",description:`Error ${h.status}: ${h.responseText}`,icon:"i-bi-alert",color:"error",duration:0,close:!0,closeIcon:"i-bi-cross"}),b.value.error=`Error ${h.status}: ${h.responseText}`)},h.onerror=()=>{console.error("Upload error"),b.value.stage=3,F.add({title:"Update failed",description:"An error occurred during the upload.",icon:"i-bi-alert",color:"error",duration:0,close:!0,closeIcon:"i-bi-cross"}),b.value.error="An error occurred during the upload."},b.value.stage=1,b.value.progress=0,h.send(b.value.firmwareFile),await new Promise(E=>{h.onloadend=()=>{E()}}),b.value.firmwareFile=null,b.value.stage!==3&&(b.value.progress=0)}async function N(){try{b.value.showFileUploadModal=!1,i.value.modals.updating=!0,await T(),b.value.stage!==3&&(b.value.stage=2)}catch(h){console.error("Firmware update failed:",h),b.value.stage=3,b.value.error=h instanceof Error?h.message:"Unknown error"}}return{autoUpdateSelfCheck:n,fetchAutoUpdateSelfCheck:o,setAutoUpdateSelfCheck:s,autoUpdate:i,resetAutoUpdateState:a,fetchAutoUpdateStatus:l,requestAutoUpdateCheck:f,setAutoUpdateBackgroundCheckInterval:d,clearAutoUpdateBackgroundCheckInterval:u,startAutoUpdate:g,abortAutoUpdateDownload:y,fileUpdate:b,uploadFirmware:T,startFirmwareUpdateFromFile:N}}),Qr=M("audio",()=>{const t=X(),e=R(void 0);async function r(){return await t.busyBar.AudioVolumeGet().then(o=>(e.value=o,o)).catch(async o=>(await O(o,"Couldn't get audio volume",!0),e.value))}async function i(n){return await t.busyBar.AudioVolumeSet({volume:n}).then(()=>(e.value?e.value.volume=n:e.value={volume:n},!0)).catch(async o=>(await O(o,"Couldn't set audio volume"),!1))}return{audio:e,fetchAudioVolume:r,setAudioVolume:i}}),ei=M("brightness",()=>{const t=X(),e=R(void 0);async function r(){return await t.busyBar.DisplayBrightnessGet().then(o=>{const a={value:o.value==="auto"?"auto":Number(o.value)};return e.value=a,a}).catch(async o=>(await O(o,"Couldn't get display brightness",!0),e.value))}async function i(n){return await t.busyBar.DisplayBrightnessSet(n).then(()=>(e.value=n,!0)).catch(async o=>(await O(o,"Couldn't set display brightness"),!1))}return{displayBrightness:e,fetchDisplayBrightness:r,setDisplayBrightness:i}});/*!
Copyright (c) 2023 Paul Miller (paulmillr.com)
The library paulmillr-qr is dual-licensed under the Apache 2.0 OR MIT license.
You can select a license of your choice.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/const oe={newline:10,reset:27};function ti(t){if(!Number.isSafeInteger(t))throw new Error(`integer expected: ${t}`)}function ri(t){if(!Number.isSafeInteger(t)||t<1||t>40)throw new Error(`Invalid version=${t}. Expected number [1..40]`)}function Z(t,e){return t.toString(2).padStart(e,"0")}function De(t,e){const r=t%e;return r>=0?r:e+r}function $(t,e){return new Array(t).fill(e)}function we(t){return t=t-(t>>>1&1431655765),t=(t&858993459)+(t>>>2&858993459),(t+(t>>>4)&252645135)*16843009>>>24}function xe(t){let e=0,r=0;for(const o of t)e=Math.max(e,o.length),r+=o.length;const i=new Uint8Array(r);let n=0;for(let o=0;o<e;o++)for(const s of t)o<s.length&&(i[n++]=s[o]);return i}function ii(){let t,e=1/0;return{add(r,i){r>=e||(t=i,e=r)},get:()=>t,score:()=>e}}function Ue(t){return{has:e=>t.includes(e),decode:e=>{if(!Array.isArray(e)||e.length&&typeof e[0]!="string")throw new Error("alphabet.decode input should be array of strings");return e.map(r=>{if(typeof r!="string")throw new Error(`alphabet.decode: not string element=${r}`);const i=t.indexOf(r);if(i===-1)throw new Error(`Unknown letter: "${r}". Allowed: ${t}`);return i})},encode:e=>{if(!Array.isArray(e)||e.length&&typeof e[0]!="number")throw new Error("alphabet.encode input should be an array of numbers");return e.map(r=>{if(ti(r),r<0||r>=t.length)throw new Error(`Digit index outside alphabet: ${r} (alphabet: ${t.length})`);return t[r]})}}}function Le(t){if(t.length!==32)throw new Error("expects 32 element matrix");const e=[1431655765,858993459,252645135,16711935,65535];for(let r=0;r<5;r++){const i=e[r]>>>0,n=1<<r,o=n<<1;for(let s=0;s<32;s+=o)for(let a=0;a<n;a++){const l=s+a,f=l+n,d=t[l]>>>0,u=t[f]>>>0,c=(d>>>n^u)&i;t[l]=(d^c<<n)>>>0,t[f]=(u^c)>>>0}}}const se=t=>1<<(t&31)>>>0,Q=(t,e)=>e===0?0:e===32?4294967295:(1<<e)-1<<t>>>0;class x{static size(e,r){if(typeof e=="number"&&(e={height:e,width:e}),!Number.isSafeInteger(e.height)&&e.height!==1/0)throw new Error(`Bitmap: invalid height=${e.height} (${typeof e.height})`);if(!Number.isSafeInteger(e.width)&&e.width!==1/0)throw new Error(`Bitmap: invalid width=${e.width} (${typeof e.width})`);return r!==void 0&&(e={width:Math.min(e.width,r.width),height:Math.min(e.height,r.height)}),e}static fromString(e){e=e.replace(/^\n+/g,"").replace(/\n+$/g,"");const r=e.split(String.fromCharCode(oe.newline)),i=r.length;let n;const o=[];for(const s of r){const a=s.split("").map(l=>{if(l==="X")return!0;if(l===" ")return!1;if(l!=="?")throw new Error(`Bitmap.fromString: unknown symbol=${l}`)});if(n!==void 0&&a.length!==n)throw new Error(`Bitmap.fromString different row sizes: width=${n} cur=${a.length}`);n=a.length,o.push(a)}return n===void 0&&(n=0),new x({height:i,width:n},o)}defined;value;tailMask;words;fullWords;height;width;constructor(e,r){const{height:i,width:n}=x.size(e);if(this.height=i,this.width=n,this.tailMask=Q(0,n&31||32),this.words=Math.ceil(n/32)|0,this.fullWords=Math.floor(n/32)|0,this.value=new Uint32Array(this.words*i),this.defined=new Uint32Array(this.value.length),r){if(r.length!==i)throw new Error(`Bitmap: data height mismatch: exp=${i} got=${r.length}`);for(let o=0;o<i;o++){const s=r[o];if(!s||s.length!==n)throw new Error(`Bitmap: data width mismatch at y=${o}: exp=${n} got=${s?.length}`);for(let a=0;a<n;a++)this.set(a,o,s[a])}}}point(e){return this.get(e.x,e.y)}isInside(e){return 0<=e.x&&e.x<this.width&&0<=e.y&&e.y<this.height}size(e){if(!e)return{height:this.height,width:this.width};const{x:r,y:i}=this.xy(e);return{height:this.height-i,width:this.width-r}}xy(e){if(typeof e=="number"&&(e={x:e,y:e}),!Number.isSafeInteger(e.x))throw new Error(`Bitmap: invalid x=${e.x}`);if(!Number.isSafeInteger(e.y))throw new Error(`Bitmap: invalid y=${e.y}`);return e.x=De(e.x,this.width),e.y=De(e.y,this.height),e}wordIndex(e,r){return r*this.words+(e>>>5)}bitIndex(e,r){return{word:this.wordIndex(e,r),bit:e&31}}isDefined(e,r){const i=this.wordIndex(e,r),n=se(e);return(this.defined[i]&n)!==0}get(e,r){const i=this.wordIndex(e,r),n=se(e);return(this.value[i]&n)!==0}maskWord(e,r,i){const{defined:n,value:o}=this;n[e]|=r,o[e]=o[e]&~r|-i&r}set(e,r,i){i!==void 0&&this.maskWord(this.wordIndex(e,r),se(e),i)}fillRectConst(e,r,i,n,o){if(i<=0||n<=0||o===void 0)return;const{value:s,defined:a,words:l}=this,f=e>>>5,d=e+i-1>>>5,u=e&31,c=e+i-1&31;for(let p=0;p<n;p++){const y=(r+p)*l;if(f===d){const g=Q(u,c-u+1);this.maskWord(y+f,g,o);continue}this.maskWord(y+f,Q(u,32-u),o);for(let g=f+1;g<d;g++)a[y+g]=4294967295,s[y+g]=o?4294967295:0;this.maskWord(y+d,Q(0,c+1),o)}}rectWords(e,r,i,n,o){for(let s=0;s<n;s++){const a=r+s;for(let l=0;l<i;){const f=e+l,{bit:d,word:u}=this.bitIndex(f,a),c=Math.min(32-d,i-l);o(u,f,l,s,c),l+=c}}}rect(e,r,i){const{x:n,y:o}=this.xy(e),{height:s,width:a}=x.size(r,this.size({x:n,y:o}));if(typeof i!="function")return this.fillRectConst(n,o,a,s,i),this;const{defined:l,value:f}=this;return this.rectWords(n,o,a,s,(d,u,c,p,y)=>{let g=0,b=f[d];for(let T=0;T<y;T++){const N=se(u+T),h=i({x:c+T,y:p},(b&N)!==0);h!==void 0&&(g|=N,b=b&~N|-h&N)}l[d]|=g,f[d]=b}),this}rectRead(e,r,i){const{x:n,y:o}=this.xy(e),{height:s,width:a}=x.size(r,this.size({x:n,y:o})),{value:l}=this;return this.rectWords(n,o,a,s,(f,d,u,c,p)=>{const y=l[f];for(let g=0;g<p;g++){const b=se(d+g);i({x:u+g,y:c},(y&b)!==0)}}),this}hLine(e,r,i){return this.rect(e,{width:r,height:1},i)}vLine(e,r,i){return this.rect(e,{width:1,height:r},i)}border(e=2,r){const i=this.height+2*e,n=this.width+2*e,o=new x({height:i,width:n});return o.rect(0,1/0,r),o.embed({x:e,y:e},this),o}embed(e,r){const{x:i,y:n}=this.xy(e),{height:o,width:s}=x.size(r.size(),this.size({x:i,y:n}));if(s<=0||o<=0)return this;const{value:a,defined:l}=this,{words:f,value:d}=r;for(let u=0;u<o;u++){const c=u*f;for(let p=0;p<s;){const y=i+p,{word:g,bit:b}=this.bitIndex(y,n+u),{word:T,bit:N}=r.bitIndex(p,u),h=Math.min(32-b,s-p),E=d[T],q=N&&T+1<c+f?d[T+1]:0,V=N?(E>>>N|q<<32-N)>>>0:E,W=Q(b,h),B=(V&Q(0,h))<<b>>>0;l[g]|=W,a[g]=a[g]&~W|B,p+=h}}return this}rectSlice(e,r=this.size()){const{x:i,y:n}=this.xy(e),{height:o,width:s}=x.size(r,this.size({x:i,y:n})),a=new x({height:o,width:s});return this.rectRead({x:i,y:n},{height:o,width:s},(l,f)=>{this.isDefined(i+l.x,n+l.y)&&a.set(l.x,l.y,f)}),a}transpose(){const{height:e,width:r,value:i,defined:n,words:o}=this,s=new x({height:r,width:e}),{words:a,value:l,defined:f,tailMask:d}=s,u=new Uint32Array(32),c=new Uint32Array(32);for(let p=0;p<e;p+=32)for(let y=0;y<o;y++){const g=Math.min(32,e-p);for(let b=0;b<g;b++){const T=this.wordIndex(32*y,p+b);u[b]=i[T],c[b]=n[T]}u.fill(0,g),c.fill(0,g),Le(u),Le(c);for(let b=0;b<32;b++){const T=y*32+b;if(T>=r)break;const N=s.wordIndex(p,T),h=p>>>5===a-1?d:4294967295;l[N]=u[b]&h,f[N]=c[b]&h}}return s}negate(){const e=this.defined.length;for(let r=0;r<e;r++)this.value[r]=~this.value[r],this.defined[r]=4294967295;return this}scale(e){if(!Number.isSafeInteger(e)||e>1024)throw new Error(`invalid scale factor: ${e}`);const{height:r,width:i}=this;return new x({height:e*r,width:e*i}).rect({x:0,y:0},1/0,({x:o,y:s})=>this.get(o/e|0,s/e|0))}clone(){const e=new x(this.size());return e.defined.set(this.defined),e.value.set(this.value),e}assertDrawn(){const{height:e,width:r,defined:i,tailMask:n,fullWords:o,words:s}=this;if(!(!e||!r))for(let a=0;a<e;a++){const l=a*s;for(let f=0;f<o;f++)if(i[l+f]!==4294967295)throw new Error("Invalid color type=undefined");if(s!==o&&(i[l+o]&n)!==n)throw new Error("Invalid color type=undefined")}}countPatternInRow(e,r,...i){if(r<=0||r>=32)throw new Error("wrong patternLen");const n=(1<<r)-1,{width:o,value:s,words:a}=this;let l=0;const f=this.wordIndex(0,e);for(let d=0,u=0;d<a;d++){const c=s[f+d],p=d===a-1&&o&31||32;for(let y=0;y<p;y++)if(u=(u<<1|c>>>y&1)&n,!(d*32+y+1<r)){for(const g of i)if(u===g){l++;break}}}return l}getRuns(e,r){const{width:i,value:n,words:o}=this;if(i===0)return;let s=0,a;const l=this.wordIndex(0,e);for(let f=0;f<o;f++){const d=n[l+f],u=f===o-1&&i&31||32;for(let c=0;c<u;c++){const p=(d&1<<c)!==0;if(p===a){s++;continue}a!==void 0&&r(s,a),a=p,s=1}}a!==void 0&&r(s,a)}popcnt(){const{height:e,width:r,words:i,fullWords:n,tailMask:o}=this;if(!e||!r)return 0;let s=0;for(let a=0;a<e;a++){const l=a*i;for(let f=0;f<n;f++)s+=we(this.value[l+f]);i!==n&&(s+=we(this.value[l+n]&o))}return s}countBoxes2x2(e){const{width:r,words:i}=this;if(r<2||(e|0)<0||e+1>=this.height)return 0;const n=this.wordIndex(0,e)|0,o=this.wordIndex(0,e+1)|0,a=(r&31)===0?2147483647:Q(0,r-1&31);let l=0;for(let f=0;f<i;f++){const d=this.value[n+f],u=this.value[o+f],c=~(d^u)>>>0,p=f+1<i?this.value[n+f+1]>>>0:0,y=~(d^(d>>>1|(p&1)<<31)>>>0)>>>0,g=f+1<i?this.value[o+f+1]>>>0:0,b=~(u^(u>>>1|(g&1)<<31)>>>0)>>>0;let T=(c&y&b)>>>0;f===i-1&&(T&=a),l+=we(T)}return l}toString(){const e=String.fromCharCode(oe.newline);let r="";for(let i=0;i<this.height;i++){let n="";for(let o=0;o<this.width;o++){const s=this.get(o,i);n+=this.isDefined(o,i)?s?"X":" ":"?"}r+=n+(i+1===this.height?"":e)}return r}toRaw(){const e=Array.from({length:this.height},()=>new Array(this.width));for(let r=0;r<this.height;r++){const i=e[r];for(let n=0;n<this.width;n++)i[n]=this.get(n,r)}return e}toASCII(){const{height:e,width:r}=this;let i="";for(let n=0;n<e;n+=2){for(let o=0;o<r;o++){const s=this.get(o,n),a=n+1>=e?!0:this.get(o,n+1);!s&&!a?i+="█":!s&&a?i+="▀":s&&!a?i+="▄":s&&a&&(i+=" ")}i+=String.fromCharCode(oe.newline)}return i}toTerm(){const e=String.fromCharCode(oe.reset),r=e+"[0m",i=e+"[1;47m  "+r,n=e+"[40m  "+r,o=String.fromCharCode(oe.newline);let s="";for(let a=0;a<this.height;a++){for(let l=0;l<this.width;l++){const f=this.get(l,a);s+=f?n:i}s+=o}return s}toSVG(e=!0){let r=`<svg viewBox="0 0 ${this.width} ${this.height}" xmlns="http://www.w3.org/2000/svg">`,i="",n;return this.rectRead(0,1/0,(o,s)=>{if(!s)return;const{x:a,y:l}=o;if(!e){r+=`<rect x="${a}" y="${l}" width="1" height="1" />`;return}let f=`M${a} ${l}`;if(n){const u=`m${a-n.x} ${l-n.y}`;u.length<=f.length&&(f=u)}const d=a<10?`H${a}`:"h-1";i+=`${f}h1v1${d}Z`,n=o}),e&&(r+=`<path d="${i}"/>`),r+="</svg>",r}toGIF(){const e=a=>[a&255,a>>>8&255],r=[...e(this.width),...e(this.height)],i=[];this.rectRead(0,1/0,(a,l)=>i.push(+(l===!0)));const n=126,o=[71,73,70,56,55,97,...r,246,0,0,255,255,255,...$(381,0),44,0,0,0,0,...r,0,7],s=Math.floor(i.length/n);for(let a=0;a<s;a++)o.push(n+1,128,...i.slice(n*a,n*(a+1)).map(l=>+l));return o.push(i.length%n+1,128,...i.slice(s*n).map(a=>+a)),o.push(1,129,0,59),new Uint8Array(o)}toImage(e=!1){const{height:r,width:i}=this.size(),n=new Uint8Array(r*i*(e?3:4));let o=0;for(let s=0;s<r;s++)for(let a=0;a<i;a++){const l=this.get(a,s)?0:255;n[o++]=l,n[o++]=l,n[o++]=l,e||(n[o++]=255)}return{height:r,width:i,data:n}}}const je=["low","medium","quartile","high"],Be=["numeric","alphanumeric","byte","kanji","eci"],ni=[26,44,70,100,134,172,196,242,292,346,404,466,532,581,655,733,815,901,991,1085,1156,1258,1364,1474,1588,1706,1828,1921,2051,2185,2323,2465,2611,2761,2876,3034,3196,3362,3532,3706],oi={low:[7,10,15,20,26,18,20,24,30,18,20,24,26,30,22,24,28,30,28,28,28,28,30,30,26,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30],medium:[10,16,26,18,24,16,18,22,22,26,30,22,22,24,24,28,28,26,26,26,26,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28],quartile:[13,22,18,26,18,24,18,22,20,24,28,26,24,20,30,24,28,28,26,30,28,30,30,30,30,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30],high:[17,28,22,16,22,28,26,26,24,28,24,28,22,24,24,30,28,28,26,28,30,24,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30]},si={low:[1,1,1,1,1,2,2,2,2,4,4,4,4,4,6,6,6,6,7,8,8,9,9,10,12,12,12,13,14,15,16,17,18,19,19,20,21,22,24,25],medium:[1,1,1,2,2,4,4,4,5,5,5,8,9,9,10,10,11,13,14,16,17,17,18,20,21,23,25,26,28,29,31,33,35,37,38,40,43,45,47,49],quartile:[1,1,2,2,4,4,6,6,8,8,8,10,12,16,12,17,16,18,21,20,23,23,25,27,29,34,34,35,38,40,43,45,48,51,53,56,59,62,65,68],high:[1,1,2,4,4,4,5,6,8,8,11,11,16,16,18,16,19,21,25,25,25,34,30,32,35,37,40,42,45,48,51,54,57,60,63,66,70,74,77,81]},P={size:{encode:t=>21+4*(t-1),decode:t=>(t-17)/4},sizeType:t=>Math.floor((t+7)/17),alignmentPatterns(t){if(t===1)return[];const e=6,r=P.size.encode(t)-e-1,i=r-e,n=Math.ceil(i/28);let o=Math.floor(i/n);o%2?o+=1:i%n*2>=n&&(o+=2);const s=[e];for(let a=1;a<n;a++)s.push(r-(n-a)*o);return s.push(r),s},ECCode:{low:1,medium:0,quartile:3,high:2},formatMask:21522,formatBits(t,e){const r=P.ECCode[t]<<3|e;let i=r;for(let n=0;n<10;n++)i=i<<1^(i>>9)*1335;return(r<<10|i)^P.formatMask},versionBits(t){let e=t;for(let r=0;r<12;r++)e=e<<1^(e>>11)*7973;return t<<12|e},alphabet:{numeric:Ue("0123456789"),alphanumerc:Ue("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:")},lengthBits(t,e){return{numeric:[10,12,14],alphanumeric:[9,11,13],byte:[8,16,16],kanji:[8,10,12],eci:[0,0,0]}[e][P.sizeType(t)]},modeBits:{numeric:"0001",alphanumeric:"0010",byte:"0100",kanji:"1000",eci:"0111"},capacity(t,e){const r=ni[t-1],i=oi[e][t-1],n=si[e][t-1],o=Math.floor(r/n)-i,s=n-r%n;return{words:i,numBlocks:n,shortBlocks:s,blockLen:o,capacity:(r-i*n)*8,total:(i+o)*n+n-s}}},Re=[(t,e)=>(t+e)%2==0,(t,e)=>e%2==0,(t,e)=>t%3==0,(t,e)=>(t+e)%3==0,(t,e)=>(Math.floor(e/2)+Math.floor(t/3))%2==0,(t,e)=>t*e%2+t*e%3==0,(t,e)=>(t*e%2+t*e%3)%2==0,(t,e)=>((t+e)%2+t*e%3)%2==0],v={tables:(t=>{const e=$(256,0),r=$(256,0);for(let i=0,n=1;i<256;i++)e[i]=n,r[n]=i,n<<=1,n&256&&(n^=t);return{exp:e,log:r}})(285),exp:t=>v.tables.exp[t],log(t){if(t===0)throw new Error(`GF.log: invalid arg=${t}`);return v.tables.log[t]%255},mul(t,e){return t===0||e===0?0:v.tables.exp[(v.tables.log[t]+v.tables.log[e])%255]},add:(t,e)=>t^e,pow:(t,e)=>v.tables.exp[v.tables.log[t]*e%255],inv(t){if(t===0)throw new Error(`GF.inverse: invalid arg=${t}`);return v.tables.exp[255-v.tables.log[t]]},polynomial(t){if(t.length==0)throw new Error("GF.polymomial: invalid length");if(t[0]!==0)return t;let e=0;for(;e<t.length-1&&t[e]==0;e++);return t.slice(e)},monomial(t,e){if(t<0)throw new Error(`GF.monomial: invalid degree=${t}`);if(e==0)return[0];let r=$(t+1,0);return r[0]=e,v.polynomial(r)},degree:t=>t.length-1,coefficient:(t,e)=>t[v.degree(t)-e],mulPoly(t,e){if(t[0]===0||e[0]===0)return[0];const r=$(t.length+e.length-1,0);for(let i=0;i<t.length;i++)for(let n=0;n<e.length;n++)r[i+n]=v.add(r[i+n],v.mul(t[i],e[n]));return v.polynomial(r)},mulPolyScalar(t,e){if(e==0)return[0];if(e==1)return t;const r=$(t.length,0);for(let i=0;i<t.length;i++)r[i]=v.mul(t[i],e);return v.polynomial(r)},mulPolyMonomial(t,e,r){if(e<0)throw new Error("GF.mulPolyMonomial: invalid degree");if(r==0)return[0];const i=$(t.length+e,0);for(let n=0;n<t.length;n++)i[n]=v.mul(t[n],r);return v.polynomial(i)},addPoly(t,e){if(t[0]===0)return e;if(e[0]===0)return t;let r=t,i=e;r.length>i.length&&([r,i]=[i,r]);let n=$(i.length,0),o=i.length-r.length,s=i.slice(0,o);for(let a=0;a<s.length;a++)n[a]=s[a];for(let a=o;a<i.length;a++)n[a]=v.add(r[a-o],i[a]);return v.polynomial(n)},remainderPoly(t,e){const r=Array.from(t);for(let i=0;i<t.length-e.length+1;i++){const n=r[i];if(n!==0)for(let o=1;o<e.length;o++)e[o]!==0&&(r[i+o]=v.add(r[i+o],v.mul(e[o],n)))}return r.slice(t.length-e.length+1,r.length)},divisorPoly(t){let e=[1];for(let r=0;r<t;r++)e=v.mulPoly(e,[1,v.pow(2,r)]);return e},evalPoly(t,e){if(e==0)return v.coefficient(t,0);let r=t[0];for(let i=1;i<t.length;i++)r=v.add(v.mul(e,r),t[i]);return r},euclidian(t,e,r){v.degree(t)<v.degree(e)&&([t,e]=[e,t]);let i=t,n=e,o=[0],s=[1];for(;2*v.degree(n)>=r;){let f=i,d=o;if(i=n,o=s,i[0]===0)throw new Error("rLast[0] === 0");n=f;let u=[0];const c=v.inv(i[0]);for(;v.degree(n)>=v.degree(i)&&n[0]!==0;){const p=v.degree(n)-v.degree(i),y=v.mul(n[0],c);u=v.addPoly(u,v.monomial(p,y)),n=v.addPoly(n,v.mulPolyMonomial(i,p,y))}if(u=v.mulPoly(u,o),s=v.addPoly(u,d),v.degree(n)>=v.degree(i))throw new Error(`Division failed r: ${n}, rLast: ${i}`)}const a=v.coefficient(s,0);if(a==0)throw new Error("sigmaTilde(0) was zero");const l=v.inv(a);return[v.mulPolyScalar(s,l),v.mulPolyScalar(n,l)]}};function ai(t){return{encode(e){const r=v.divisorPoly(t),i=Array.from(e);return i.push(...r.slice(0,-1).fill(0)),Uint8Array.from(v.remainderPoly(i,r))},decode(e){const r=e.slice(),i=v.polynomial(Array.from(e));let n=$(t,0),o=!1;for(let u=0;u<t;u++){const c=v.evalPoly(i,v.exp(u));n[n.length-1-u]=c,c!==0&&(o=!0)}if(!o)return r;n=v.polynomial(n);const s=v.monomial(t,1),[a,l]=v.euclidian(s,n,t),f=$(v.degree(a),0);let d=0;for(let u=1;u<256&&d<f.length;u++)v.evalPoly(a,u)===0&&(f[d++]=v.inv(u));if(d!==f.length)throw new Error("RS.decode: invalid errors number");for(let u=0;u<f.length;u++){const c=r.length-1-v.log(f[u]);if(c<0)throw new Error("RS.decode: invalid error location");const p=v.inv(f[u]);let y=1;for(let g=0;g<f.length;g++)u!==g&&(y=v.mul(y,v.add(1,v.mul(f[g],p))));r[c]=v.add(r[c],v.mul(v.evalPoly(l,p),v.inv(y)))}return r}}}function ui(t,e){const{words:r,shortBlocks:i,numBlocks:n,blockLen:o,total:s}=P.capacity(t,e),a=ai(r);return{encode(l){const f=[],d=[];for(let y=0;y<n;y++){const g=y<i,b=o+(g?0:1);f.push(l.subarray(0,b)),d.push(a.encode(l.subarray(0,b))),l=l.subarray(b)}const u=xe(f),c=xe(d),p=new Uint8Array(u.length+c.length);return p.set(u),p.set(c,u.length),p},decode(l){if(l.length!==s)throw new Error(`interleave.decode: len(data)=${l.length}, total=${s}`);const f=[];for(let c=0;c<n;c++){const p=c<i;f.push(new Uint8Array(r+o+(p?0:1)))}let d=0;for(let c=0;c<o;c++)for(let p=0;p<n;p++)f[p][c]=l[d++];for(let c=i;c<n;c++)f[c][o]=l[d++];for(let c=o;c<o+r;c++)for(let p=0;p<n;p++){const y=p<i;f[p][c+(y?0:1)]=l[d++]}const u=[];for(const c of f)u.push(...Array.from(a.decode(c)).slice(0,-r));return Uint8Array.from(u)}}}function li(t,e,r,i=!1){const n=P.size.encode(t);let o=new x(n+2);const s=new x(3).rect(0,3,!0).border(1,!1).border(1,!0).border(1,!1);o=o.embed(0,s).embed({x:-s.width,y:0},s).embed({x:0,y:-s.height},s),o=o.rectSlice(1,n);const a=new x(1).rect(0,1,!0).border(1,!1).border(1,!0),l=P.alignmentPatterns(t);for(const f of l)for(const d of l)o.isDefined(d,f)||o.embed({x:d-2,y:f-2},a);o=o.hLine({x:0,y:6},1/0,({x:f})=>o.isDefined(f,6)?void 0:f%2==0).vLine({x:6,y:0},1/0,({y:f})=>o.isDefined(6,f)?void 0:f%2==0);{const f=P.formatBits(e,r),d=u=>!i&&(f>>u&1)==1;for(let u=0;u<6;u++)o.set(8,u,d(u));for(let u=6;u<8;u++)o.set(8,u+1,d(u));for(let u=8;u<15;u++)o.set(8,n-15+u,d(u));for(let u=0;u<8;u++)o.set(n-u-1,8,d(u));for(let u=8;u<9;u++)o.set(15-u-1+1,8,d(u));for(let u=9;u<15;u++)o.set(15-u-1,8,d(u));o.set(8,n-8,!i)}if(t>=7){const f=P.versionBits(t);for(let d=0;d<18;d+=1){const u=!i&&(f>>d&1)==1,c=Math.floor(d/3),p=d%3+n-8-3;o.set(p,c,u),o.set(c,p,u)}}return o}function ci(t,e,r){const i=t.height,n=Re[e];let o=-1,s=i-1;for(let a=i-1;a>0;a-=2){for(a==6&&(a=5);;s+=o){for(let l=0;l<2;l+=1){const f=a-l;t.isDefined(f,s)||r(f,s,n(f,s))}if(s+o<0||s+o>=i)break}o=-o}}function fi(t){let e="numeric";for(let r of t)if(!P.alphabet.numeric.has(r)&&(e="alphanumeric",!P.alphabet.alphanumerc.has(r)))return"byte";return e}function di(t){if(typeof t!="string")throw new Error(`utf8ToBytes expected string, got ${typeof t}`);return new Uint8Array(new TextEncoder().encode(t))}function Fe(t,e,r,i,n=di){let o="",s=r.length;if(i==="numeric"){const c=P.alphabet.numeric.decode(r.split("")),p=c.length;for(let y=0;y<p-2;y+=3)o+=Z(c[y]*100+c[y+1]*10+c[y+2],10);p%3===1?o+=Z(c[p-1],4):p%3===2&&(o+=Z(c[p-2]*10+c[p-1],7))}else if(i==="alphanumeric"){const c=P.alphabet.alphanumerc.decode(r.split("")),p=c.length;for(let y=0;y<p-1;y+=2)o+=Z(c[y]*45+c[y+1],11);p%2==1&&(o+=Z(c[p-1],6))}else if(i==="byte"){const c=n(r);s=c.length,o=Array.from(c).map(p=>Z(p,8)).join("")}else throw new Error("encode: unsupported type");const{capacity:a}=P.capacity(t,e),l=Z(s,P.lengthBits(t,i));let f=P.modeBits[i]+l+o;if(f.length>a)throw new Error("Capacity overflow");f+="0".repeat(Math.min(4,Math.max(0,a-f.length))),f.length%8&&(f+="0".repeat(8-f.length%8));const d="1110110000010001";for(let c=0;f.length!==a;c++)f+=d[c%d.length];const u=Uint8Array.from(f.match(/(.{8})/g).map(c=>+`0b${c}`));return ui(t,e).encode(u)}function Me(t,e,r,i,n=!1){const o=li(t,e,i,n);let s=0;const a=8*r.length;if(ci(o,i,(l,f,d)=>{let u=!1;s<a&&(u=(r[s>>>3]>>(7-s&7)&1)!==0,s++),o.set(l,f,u!==d)}),s!==a)throw new Error("QR: bytes left after draw");return o}const Ze=t=>{const e=t.map(r=>r?"1":"0").join("");return{len:e.length,n:+`0b${e}`}},Qe=[!0,!1,!0,!0,!0,!1,!0],et=[!1,!1,!1,!1],ce=Ze([...Qe,...et]),We=Ze([...et,...Qe]);function pi(t){const{width:e,height:r}=t,i=t.transpose();let n=0;for(let d=0;d<r;d++)t.getRuns(d,u=>{u>=5&&(n+=3+(u-5))});for(let d=0;d<e;d++)i.getRuns(d,u=>{u>=5&&(n+=3+(u-5))});let o=0;for(let d=0;d<r-1;d++)o+=3*t.countBoxes2x2(d);let s=0;for(let d=0;d<r;d++)s+=40*t.countPatternInRow(d,ce.len,ce.n,We.n);for(let d=0;d<e;d++)s+=40*i.countPatternInRow(d,ce.len,ce.n,We.n);let a=0;a=t.popcnt();const l=a/(r*e)*100,f=10*Math.floor(Math.abs(l-50)/5);return n+o+s+f}function hi(t,e,r,i){if(i===void 0){const n=ii();for(let o=0;o<Re.length;o++)n.add(pi(Me(t,e,r,o,!0)),o);i=n.get()}if(i===void 0)throw new Error("Cannot find mask");return Me(t,e,r,i)}function mi(t){if(!je.includes(t))throw new Error(`Invalid error correction mode=${t}. Expected: ${je}`)}function yi(t){if(!Be.includes(t))throw new Error(`Encoding: invalid mode=${t}. Expected: ${Be}`);if(t==="kanji"||t==="eci")throw new Error(`Encoding: ${t} is not supported (yet?).`)}function vi(t){if(![0,1,2,3,4,5,6,7].includes(t)||!Re[t])throw new Error(`Invalid mask=${t}. Expected number [0..7]`)}function gi(t,e="raw",r={}){const i=r.ecc!==void 0?r.ecc:"medium";mi(i);const n=r.encoding!==void 0?r.encoding:fi(t);yi(n),r.mask!==void 0&&vi(r.mask);let o=r.version,s,a=new Error("Unknown error");if(o!==void 0)ri(o),s=Fe(o,i,t,n,r.textEncoder);else for(let d=1;d<=40;d++)try{s=Fe(d,i,t,n,r.textEncoder),o=d;break}catch(u){a=u}if(!o||!s)throw a;let l=hi(o,i,s,r.mask);l.assertDrawn();const f=r.border===void 0?2:r.border;if(!Number.isSafeInteger(f))throw new Error(`invalid border type=${typeof f}`);if(l=l.border(f,!1),r.scale!==void 0&&(l=l.scale(r.scale)),e==="raw")return l.toRaw();if(e==="ascii")return l.toASCII();if(e==="svg")return l.toSVG(r.optimize);if(e==="gif")return l.toGIF();if(e==="term")return l.toTerm();throw new Error(`Unknown output: ${e}`)}const bi=M("matter",()=>{const t=X(),e=R({fabricCount:0,latestStatus:void 0}),r=R({qrCodeMatrix:null,manualCode:"",availableUntil:null,showModal:!1,expiresInMs:0,timeout:null});async function i(){await t.busyBar.SmartHomePairingGet().then(s=>{e.value.fabricCount=s.fabric_count||0,e.value.latestStatus=s.latest_pairing_status}).catch(async s=>{await O(s,"Couldn't get Matter commissioning status",!0)})}async function n(){await t.busyBar.SmartHomePair().then(s=>{r.value.manualCode=s.manual_code||"",r.value.availableUntil=new Date(Number(s.available_until)),r.value.timeout&&clearTimeout(r.value.timeout),r.value.expiresInMs=r.value.availableUntil.getTime()-Date.now(),r.value.qrCodeMatrix=gi(s.qr_code,"raw"),r.value.timeout=setTimeout(()=>{r.value.showModal=!1,r.value.qrCodeMatrix=null,r.value.manualCode="",r.value.availableUntil=null,r.value.timeout=null,console.debug("Matter commissioning link expired")},r.value.expiresInMs),r.value.showModal=!0}).catch(async s=>{await O(s,"Couldn't request Matter commissioning link")})}async function o(){await t.busyBar.SmartHomeErase().then(()=>{console.debug("All Matter pairings deleted, waiting for device to reboot")}).catch(async s=>{console.error(s.message),!s.message.includes("timed out")&&await O(s,"Couldn't delete pairings")})}return{matterCommissioning:e,matterLink:r,fetchMatterCommissioning:i,requestMatterLink:n,deleteAllPairings:o}}),wi=M("timezone",()=>{const t=X(),e=R(void 0);async function r(){return await t.busyBar.TimeTimezoneGet().then(o=>(e.value=o.name,o.name)).catch(async o=>(await O(o,"Couldn't get timezone",!0),e.value))}async function i(n){return await t.busyBar.TimeTimezoneSet({timezone:n}).then(()=>(e.value=n,!0)).catch(async o=>(await O(o,"Couldn't set timezone"),!1))}return{timezone:e,fetchTimezone:r,setTimezone:i}}),Ei=M("screenStream",()=>({currentFrame:R(null)}),{persist:!1});function me(t){return!!t&&typeof t=="object"&&!Array.isArray(t)&&!(t instanceof Uint8Array)}function Ai(t){return me(t)&&"from"in t&&"to"in t&&Object.keys(t).length===2}function fe(t){if(typeof t=="string")return t;if(t===void 0)return"undefined";if(t===null)return"null";if(typeof t=="number"||typeof t=="boolean"||typeof t=="bigint")return String(t);try{return JSON.stringify(t)}catch{return String(t)}}function _e(t,e){if(t!==e){if(me(t)&&me(e)){const r=new Set([...Object.keys(t),...Object.keys(e)]),i={};for(const n of r){const o=_e(t[n],e[n]);o!==void 0&&(i[n]=o)}return Object.keys(i).length?i:void 0}if(Array.isArray(t)&&Array.isArray(e)){const r=Math.max(t.length,e.length),i=[];let n=!1;for(let o=0;o<r;o++){const s=_e(t[o],e[o]);s!==void 0&&(i[o]=s,n=!0)}return n?i:void 0}return{from:t,to:e}}}function Te(t,e=""){return t===void 0?[]:Ai(t)?[`${e||"value"}: ${fe(t.from)} -> ${fe(t.to)}`]:Array.isArray(t)?t.flatMap((r,i)=>Te(r,`${e}[${i}]`)):me(t)?Object.entries(t).flatMap(([r,i])=>{const n=e?`${e}.${r}`:r;return Te(i,n)}):e?[`${e}: ${fe(t)}`]:[fe(t)]}function _i(t){switch(t){case k.WifiSecurity.OPEN:return"Open";case k.WifiSecurity.WPA:return"WPA";case k.WifiSecurity.WPA2:return"WPA2";case k.WifiSecurity.WEP:return"WEP";case k.WifiSecurity.WPA_WPA2:return"WPA/WPA2";case k.WifiSecurity.WPA3:return"WPA3";case k.WifiSecurity.WPA2_WPA3:return"WPA2/WPA3";default:return}}function Ti(t){switch(t){case k.IpConfigurationMethod.DHCP:return"dhcp";case k.IpConfigurationMethod.STATIC:return"static";default:return}}function Ri(t){switch(t){case k.IpProtocol.IPV4:return"ipv4";case k.IpProtocol.IPV6:return"ipv6";default:return}}function Si(t){if(t){if(t.includes(":"))return"ipv6";if(t.includes("."))return"ipv4"}}function Oi(t){switch(t){case k.MatterCommissioningStatus.NEVER_STARTED:return"never_started";case k.MatterCommissioningStatus.STARTED:return"started";case k.MatterCommissioningStatus.COMPLETED_SUCCESSFULLY:return"completed_successfully";case k.MatterCommissioningStatus.FAILED:return"failed";default:return}}const tt=M("stateStream",()=>{const t=ie(),e=X(),r=Qr(),i=ei(),n=Ae(),o=bi(),s=wi(),a=rt(),l=Ei(),f=ye().public.barUrl||window.location.origin,d=ue(),u=R(!1),c=R(!1),p=R(!1),y=$e(new Kr({addr:f,token:t.apiKey||""},{timeout:Number(d.get("stateStreamTimeout")),dataTimeout:Number(d.get("stateStreamDataTimeout")),maxReconnectAttempts:Number(d.get("stateStreamMaxReconnectAttempts")),reconnectDelay:Number(d.get("stateStreamReconnectDelay"))})),g=R(null),b=R(!0);function T(){y.value.stop(),g.value?.data.status===j.STALE&&(g.value.data.status=j.NONE),b.value=!0}function N(A){const m=A.name;m&&(e.deviceName=m)}function h(A){const m=A.known,w=m?{state:m.batteryStatus?k.BatteryStatus[m.batteryStatus]:void 0,battery_charge:m.batteryChargePercent??0,battery_voltage:m.batteryVoltageMv??0,battery_current:m.batteryCurrentMa??0,usb_voltage:m.usbVoltageMv??0}:void 0;w&&(e.deviceStatus={...e.deviceStatus,power:w})}function E(A){if(A.automatic){i.displayBrightness={value:"auto"};return}const m=A.manual?.brightness;m!==void 0?i.displayBrightness={value:m}:i.displayBrightness={value:0}}function q(A){const m=A.volume;m!==void 0?r.audio={volume:m??0}:r.audio={volume:0}}function V(A){const m=Object.keys(A).filter(ge=>ge!=="ipAddresses")[0],w=A.connected,U=(Array.isArray(A.ipAddresses)?A.ipAddresses:[]).find(ge=>ge.address),I=a.wifi?.ip_config,C=U?U.address:void 0;let G=m;m==="connected"&&w?.status===k.WifiConnectionStatus.RECONNECTING&&(G="reconnecting");const J={state:G,ssid:w?.ssid,bssid:w?.bssid,channel:w?.channel,rssi:w?.rssi,security:_i(w?.security),ip_config:U?{ip_method:Ti(U.method)??I?.ip_method,ip_type:Ri(U.protocol)??Si(C)??I?.ip_type,address:C,gateway:U.gateway,mask:U.netmask}:void 0},K=a.wifi?.state;a.wifi=J,K!==J.state&&(J.state==="connected"?window.dispatchEvent(new Event("wifi-reconnected")):K==="connected"&&window.dispatchEvent(new Event("wifi-disconnected")))}function W(A){if(n.autoUpdate.isChecking=!1,A.available){n.autoUpdate.status="available",n.autoUpdate.availableVersion=A.available?.version??null,n.autoUpdate.isAllowed=!0;return}const m=A.unavailable?.reason;if(n.autoUpdate.availableVersion=null,m===he.CheckError.NOT_AVAILABLE){n.autoUpdate.status="not_available",n.autoUpdate.isAllowed=!0;return}if(m===he.CheckError.FAILURE){n.autoUpdate.status="failure",n.autoUpdate.isAllowed=!1;return}n.autoUpdate.status=null}function B(A){A.name&&(s.timezone=A.name)}function z(A){const m=A.state;o.matterCommissioning={fabricCount:A.fabricCount??0,latestStatus:m?{value:Oi(m.status),timestamp:m.timestamp??0}:void 0}}function ne(A){if(A.updates)for(const m of A.updates)switch(m.state){case"deviceName":m.deviceName&&N(m.deviceName);break;case"power":m.power&&h(m.power);break;case"brightness":m.brightness&&E(m.brightness);break;case"audioVolume":m.audioVolume&&q(m.audioVolume);break;case"wifi":m.wifi&&V(m.wifi);break;case"updateCheck":m.updateCheck&&W(m.updateCheck);break;case"timezone":m.timezone&&B(m.timezone);break;case"matter":m.matter&&z(m.matter);break;case"frame":m.frame&&(l.currentFrame=m.frame);break}}async function ee(A){const m=g.value;if(m===null){g.value=A,d.get("stateStreamLogStatusUpdates")&&console.debug("[state stream status] Initial stream status:",A);return}const w=_e(m,A);if(w)for(const _ of Te(w))d.get("stateStreamLogStatusUpdates")&&console.debug("[state stream status]",_,"| full status:",A);g.value=A,g.value.data.status===j.STALE&&m?.data.status!==j.STALE&&b.value&&(console.debug("No state messages received for a while, checking connection..."),await e.checkConnection()===!1&&(console.debug("Connection check failed after state stream data stale, stopping stream and starting polling"),T(),e.setRefreshInterval())),g.value.data.status===j.ACTIVE&&g.value.connection.status===Y.CONNECTED&&g.value.main.status===D.RUNNING&&(p.value&&(console.debug("Stream is active again, hiding resource limit error banner"),p.value=!1),c.value&&(console.debug("Stream is active again, hiding state stream failure banner"),c.value=!1))}return{streamNotRestartable:u,showStateStreamFailBanner:c,showResourceLimitErrorBanner:p,streamStatus:g,stream:y,doCheckConnectionOnStreamDataStale:b,stopStream:T,applyStateMessage:ne,applyStreamStatus:ee}}),X=M("device",()=>{const t=ie().apiRequest,e=rt(),r=Ae(),i=tt(),n=ue(),o=$e(new Je({addr:ye().public.barUrl||window.location.origin,timeout:Number(n.get("httpRequestTimeout"))})),s=R(!0),a=R(!1),l=R(0);async function f(){if(a.value)return"aborted";s.value&&l.value>=3&&(console.warn("Data has been stale for a while and multiple connection checks have succeeded, restarting state stream as it seems to be in a bad state"),i.stopStream(),l.value=0,window.dispatchEvent(new Event("protobuf-websocket-restart"))),a.value=!0;const m=s.value;try{await t("/api/name",{timeout:Number(n.get("httpRequestTimeout"))}),s.value||(window.dispatchEvent(new Event("device-reconnected")),r.autoUpdate.stage===H.UPDATING&&(r.autoUpdate.stage=H.SUCCESS)),s.value=!0,console.debug("Device is connected"),F.remove("device-disconnected")}catch(w){if(!d.value&&i.streamStatus?.data.status===j.ACTIVE){console.debug("conncheck request aborted, ignoring because refresh interval is cleared and stream data is active");const _=w;if(_?.name==="AbortError"||_?.message?.toLowerCase().includes("abort")||_?.code==="ECONNABORTED")return a.value=!1,"aborted"}m&&window.dispatchEvent(new Event("device-disconnected")),s.value=!1,console.debug("Device is disconnected"),r.autoUpdate.stage!==H.UPDATING&&!(r.autoUpdate.stage===H.SUCCESS&&e.wifi?.state!=="connected")&&(r.fileUpdate.stage===H.IDLE||r.fileUpdate.stage===H.ERROR)&&F.add({id:"device-disconnected",title:"Device disconnected",description:"Device lost. Please check the connection.",icon:"i-bi-alert",color:"error",duration:0,close:!0,closeIcon:"i-bi-cross"})}return a.value=!1,s.value&&i.streamStatus?.data.status===j.STALE?l.value++:l.value=0,s.value}const d=R();async function u(){if(n.get("refreshDeviceDataAbortIfStreamActive")&&(console.debug("Checking whether to refresh device data. Stream status:",i.streamStatus),i.streamStatus?.main.status===D.RUNNING&&i.streamStatus?.data.status===j.ACTIVE)){console.debug("Skipping device data refresh because stream is active and config is set to abort in this case"),d.value&&(clearInterval(d.value),d.value=void 0,console.debug("Cleared refresh interval to stop refreshing device data while stream is active"));return}const m=Ae();if(m.autoUpdate.stage===H.LOADING||m.fileUpdate.stage===H.LOADING){console.debug("Skipping connection check during auto update");return}if(await f(),!s.value)return;F.remove("device-disconnected"),await h();const w=e.wifi?.state,_=await e.fetchWifiState();w!==_?.state&&(_?.state==="connected"?window.dispatchEvent(new Event("wifi-reconnected")):window.dispatchEvent(new Event("wifi-disconnected"))),await z()}function c(){d.value=Ee(u,Number(n.get("httpPollingInterval")))}function p(){d.value&&(clearInterval(d.value),d.value=void 0)}const y=R("wifi");async function g(){await o.value.SystemTransportGet().then(m=>{y.value=m.type,console.debug("Detected connection type:",y.value)}).catch(async m=>(await O(m,"Couldn't get connection type",!0),y.value))}const b=R(void 0);async function T(){return await o.value.SystemVersionGet().then(w=>(b.value=w,w)).catch(async w=>(await O(w,"Couldn't get HTTP API version",!0),b.value))}const N=R(void 0);async function h(){return await o.value.SystemStatusGet().then(w=>(N.value=w,w)).catch(async w=>(await O(w,"Couldn't get device status",!0),N.value))}const E="BUSY Bar",q=R(void 0);async function V(m=!1){return await o.value.SettingsNameGet().then(_=>(q.value=_.name,_.name)).catch(async _=>{if(m)throw _;return await O(_,"Couldn't get device name"),E})}async function W(m){return await o.value.SettingsNameSet({name:m}).then(()=>(q.value=m,F.add({title:"Changes saved",icon:"i-bi-checkmark-circle-fill",color:"success"}),!0)).catch(async w=>(await O(w,"Couldn't set device name"),!1))}const B=R(void 0);async function z(){return await o.value.SettingsAccessGet().then(w=>(B.value=w,w)).catch(async w=>(await O(w,"Couldn't get HTTP API access state",!0),B.value))}async function ne(m,w){const _={mode:m};if(m==="key"){if(!w)throw new Error("Password not provided");_.key=w}return await o.value.SettingsAccessSet(_).then(async()=>(B.value=await z(),F.add({title:m==="key"?"Password set":"Changes saved",icon:"i-bi-checkmark-circle-fill",color:"success"}),!0)).catch(async U=>(await O(U,"Couldn't set HTTP API access state"),!1))}const ee=R(null);async function A(){return await o.value.AccountInfoGet().then(w=>(ee.value=w,w)).catch(async w=>(await O(w,"Couldn't get account info",!0),null))}return{busyBar:o,isConnected:s,checkConnection:f,connectionType:y,detectConnectionType:g,refreshInterval:d,setRefreshInterval:c,clearRefreshInterval:p,apiVersion:b,fetchApiVersion:T,deviceStatus:N,fetchDeviceStatus:h,deviceName:q,fetchDeviceName:V,setDeviceName:W,httpAPIAccess:B,fetchHttpAPIAccess:z,setHttpAPIAccess:ne,accountInfo:ee,fetchAccountInfo:A}}),rt=M("wifi",()=>{const t=X(),e=tt(),r=ue(),i=R(void 0);async function n(){return await t.busyBar.WifiStatusGet().then(c=>(i.value=c,c)).catch(async c=>(await O(c,"Couldn't fetch WiFi state",!0),i.value))}const o=R(!1);async function s(){if(o.value){if(r.get("wifiAbortSimultaneousRequests"))return console.warn("wifiStore.listWifiNetworks: already loading, skipping"),[];console.debug("wifiStore.listWifiNetworks: already loading, but wifiAbortSimultaneousRequests is false, allowing simultaneous request")}o.value=!0;const u=t.refreshInterval;return u&&t.clearRefreshInterval(),e.doCheckConnectionOnStreamDataStale=!1,await t.busyBar.WifiNetworksGet({timeout:0}).then(c=>{if(!c||!Array.isArray(c.networks))throw new Error("Failed to fetch WiFi networks");return c.networks=c.networks.reduce((p,y)=>{const g=p.find(b=>b.ssid===y.ssid);if(!g)p.push(y);else if(y.rssi&&g.rssi&&y.rssi<g.rssi){const b=p.indexOf(g);p[b]=y}return p},[]),c.networks}).catch(async c=>{if(i.value?.state!=="connected")return await O(c,"Couldn't list WiFi networks",!1,0),[]}).finally(()=>{e.doCheckConnectionOnStreamDataStale=!0,u&&(console.debug("wifiStore.listWifiNetworks: was polling before, resuming polling"),t.setRefreshInterval()),o.value=!1})}const a=R(!1);async function l(u){if(a.value){if(r.get("wifiAbortSimultaneousRequests"))return console.warn("wifiStore.connectToWifiNetwork: already connecting, skipping"),!1;console.debug("wifiStore.connectToWifiNetwork: already connecting, but wifiAbortSimultaneousRequests is false, allowing simultaneous request")}a.value=!0;const c=t.refreshInterval;return c&&t.clearRefreshInterval(),e.doCheckConnectionOnStreamDataStale=!1,await t.busyBar.WifiConnect({...u,timeout:0}).catch(async p=>(await O(p,"Couldn't connect to WiFi network",!1,0),!1)).finally(()=>{e.doCheckConnectionOnStreamDataStale=!0,c&&(console.debug("wifiStore.connectToWifiNetwork: was polling before, resuming polling"),t.setRefreshInterval()),a.value=!1})}const f=R(!1);async function d(){if(f.value){if(r.get("wifiAbortSimultaneousRequests"))return console.warn("wifiStore.disconnectFromWifiNetwork: already disconnecting, skipping"),!1;console.debug("wifiStore.disconnectFromWifiNetwork: already disconnecting, but wifiAbortSimultaneousRequests is false, allowing simultaneous request")}f.value=!0;const u=t.refreshInterval;return u&&t.clearRefreshInterval(),e.doCheckConnectionOnStreamDataStale=!1,await t.busyBar.WifiDisconnect({timeout:0}).catch(async c=>(await O(c,"Couldn't disconnect from WiFi network",!1,0),!1)).finally(()=>{e.doCheckConnectionOnStreamDataStale=!0,u&&(console.debug("wifiStore.disconnectFromWifiNetwork: was polling before, resuming polling"),t.setRefreshInterval()),f.value=!1})}return{wifi:i,fetchWifiState:n,listWifiNetworks:s,connectToWifiNetwork:l,disconnectFromWifiNetwork:d}});export{Pe as D,Ci as N,H as U,X as a,Ae as b,Qr as c,ei as d,ue as e,wi as f,bi as g,O as h,ie as i,Ei as j,ke as k,tt as l,L as m,D as n,Ee as s,F as t,rt as u};
