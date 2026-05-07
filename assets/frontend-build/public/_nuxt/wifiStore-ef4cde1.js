import{aF as Qe,aq as V,ar as et,aP as me,q as O,aO as tt,aV as Me}from"./entry-ef4cde1.js";const ue=Qe(),L={...ue,add:t=>{const e=ue.toasts.value.find(r=>r.id===t.id);return e?(ue.update(t.id,t),e):ue.add(t)}},rt=/\{[^{}]+\}/g,it=()=>typeof process=="object"&&Number.parseInt(process?.versions?.node?.substring(0,2))>=18&&process.versions.undici;function nt(){return Math.random().toString(36).slice(2,11)}function We(t){let{baseUrl:e="",Request:r=globalThis.Request,fetch:i=globalThis.fetch,querySerializer:n,bodySerializer:o,headers:s,requestInitExt:a=void 0,...u}={...t};a=it()?a:void 0,e=Oe(e);const c=[];async function d(l,f){const{baseUrl:h,fetch:g=i,Request:m=r,headers:E,params:_={},parseAs:p="json",querySerializer:w,bodySerializer:P=o??st,body:B,...F}=f||{};let G=e;h&&(G=Oe(h)??e);let A=typeof n=="function"?n:Te(n);w&&(A=typeof w=="function"?w:Te({...typeof n=="object"?n:{},...w}));const b=B===void 0?void 0:P(B,Re(s,E,_.header)),y=Re(b===void 0||b instanceof FormData?{}:{"Content-Type":"application/json"},s,E,_.header),I={redirect:"follow",...u,...F,body:b,headers:y};let H,U,M=new m(at(l,{baseUrl:G,params:_,querySerializer:A}),I),S;for(const k in F)k in M||(M[k]=F[k]);if(c.length){H=nt(),U=Object.freeze({baseUrl:G,fetch:g,parseAs:p,querySerializer:A,bodySerializer:P});for(const k of c)if(k&&typeof k=="object"&&typeof k.onRequest=="function"){const N=await k.onRequest({request:M,schemaPath:l,params:_,options:U,id:H});if(N)if(N instanceof m)M=N;else if(N instanceof Response){S=N;break}else throw new Error("onRequest: must return new Request() or Response() when modifying the request")}}if(!S){try{S=await g(M,a)}catch(k){let N=k;if(c.length)for(let W=c.length-1;W>=0;W--){const ae=c[W];if(ae&&typeof ae=="object"&&typeof ae.onError=="function"){const re=await ae.onError({request:M,error:N,schemaPath:l,params:_,options:U,id:H});if(re){if(re instanceof Response){N=void 0,S=re;break}if(re instanceof Error){N=re;continue}throw new Error("onError: must return new Response() or instance of Error")}}}if(N)throw N}if(c.length)for(let k=c.length-1;k>=0;k--){const N=c[k];if(N&&typeof N=="object"&&typeof N.onResponse=="function"){const W=await N.onResponse({request:M,response:S,schemaPath:l,params:_,options:U,id:H});if(W){if(!(W instanceof Response))throw new Error("onResponse: must return new Response() when modifying the response");S=W}}}}if(S.status===204||M.method==="HEAD"||S.headers.get("Content-Length")==="0")return S.ok?{data:void 0,response:S}:{error:void 0,response:S};if(S.ok)return p==="stream"?{data:S.body,response:S}:{data:await S[p](),response:S};let Z=await S.text();try{Z=JSON.parse(Z)}catch{}return{error:Z,response:S}}return{request(l,f,h){return d(f,{...h,method:l.toUpperCase()})},GET(l,f){return d(l,{...f,method:"GET"})},PUT(l,f){return d(l,{...f,method:"PUT"})},POST(l,f){return d(l,{...f,method:"POST"})},DELETE(l,f){return d(l,{...f,method:"DELETE"})},OPTIONS(l,f){return d(l,{...f,method:"OPTIONS"})},HEAD(l,f){return d(l,{...f,method:"HEAD"})},PATCH(l,f){return d(l,{...f,method:"PATCH"})},TRACE(l,f){return d(l,{...f,method:"TRACE"})},use(...l){for(const f of l)if(f){if(typeof f!="object"||!("onRequest"in f||"onResponse"in f||"onError"in f))throw new Error("Middleware must be an object with one of `onRequest()`, `onResponse() or `onError()`");c.push(f)}},eject(...l){for(const f of l){const h=c.indexOf(f);h!==-1&&c.splice(h,1)}}}}function ye(t,e,r){if(e==null)return"";if(typeof e=="object")throw new Error("Deeply-nested arrays/objects aren’t supported. Provide your own `querySerializer()` to handle these.");return`${t}=${r?.allowReserved===!0?e:encodeURIComponent(e)}`}function Ge(t,e,r){if(!e||typeof e!="object")return"";const i=[],n={simple:",",label:".",matrix:";"}[r.style]||"&";if(r.style!=="deepObject"&&r.explode===!1){for(const a in e)i.push(a,r.allowReserved===!0?e[a]:encodeURIComponent(e[a]));const s=i.join(",");switch(r.style){case"form":return`${t}=${s}`;case"label":return`.${s}`;case"matrix":return`;${t}=${s}`;default:return s}}for(const s in e){const a=r.style==="deepObject"?`${t}[${s}]`:s;i.push(ye(a,e[s],r))}const o=i.join(n);return r.style==="label"||r.style==="matrix"?`${n}${o}`:o}function $e(t,e,r){if(!Array.isArray(e))return"";if(r.explode===!1){const o={form:",",spaceDelimited:"%20",pipeDelimited:"|"}[r.style]||",",s=(r.allowReserved===!0?e:e.map(a=>encodeURIComponent(a))).join(o);switch(r.style){case"simple":return s;case"label":return`.${s}`;case"matrix":return`;${t}=${s}`;default:return`${t}=${s}`}}const i={simple:",",label:".",matrix:";"}[r.style]||"&",n=[];for(const o of e)r.style==="simple"||r.style==="label"?n.push(r.allowReserved===!0?o:encodeURIComponent(o)):n.push(ye(t,o,r));return r.style==="label"||r.style==="matrix"?`${i}${n.join(i)}`:n.join(i)}function Te(t){return function(r){const i=[];if(r&&typeof r=="object")for(const n in r){const o=r[n];if(o!=null){if(Array.isArray(o)){if(o.length===0)continue;i.push($e(n,o,{style:"form",explode:!0,...t?.array,allowReserved:t?.allowReserved||!1}));continue}if(typeof o=="object"){i.push(Ge(n,o,{style:"deepObject",explode:!0,...t?.object,allowReserved:t?.allowReserved||!1}));continue}i.push(ye(n,o,t))}}return i.join("&")}}function ot(t,e){let r=t;for(const i of t.match(rt)??[]){let n=i.substring(1,i.length-1),o=!1,s="simple";if(n.endsWith("*")&&(o=!0,n=n.substring(0,n.length-1)),n.startsWith(".")?(s="label",n=n.substring(1)):n.startsWith(";")&&(s="matrix",n=n.substring(1)),!e||e[n]===void 0||e[n]===null)continue;const a=e[n];if(Array.isArray(a)){r=r.replace(i,$e(n,a,{style:s,explode:o}));continue}if(typeof a=="object"){r=r.replace(i,Ge(n,a,{style:s,explode:o}));continue}if(s==="matrix"){r=r.replace(i,`;${ye(n,a)}`);continue}r=r.replace(i,s==="label"?`.${encodeURIComponent(a)}`:encodeURIComponent(a))}return r}function st(t,e){return t instanceof FormData?t:e&&(e.get instanceof Function?e.get("Content-Type")??e.get("content-type"):e["Content-Type"]??e["content-type"])==="application/x-www-form-urlencoded"?new URLSearchParams(t).toString():JSON.stringify(t)}function at(t,e){let r=`${e.baseUrl}${t}`;e.params?.path&&(r=ot(r,e.params.path));let i=e.querySerializer(e.params.query??{});return i.startsWith("?")&&(i=i.substring(1)),i&&(r+=`?${i}`),r}function Re(...t){const e=new Headers;for(const r of t){if(!r||typeof r!="object")continue;const i=r instanceof Headers?r.entries():Object.entries(r);for(const[n,o]of i)if(o===null)e.delete(n);else if(Array.isArray(o))for(const s of o)e.append(n,s);else o!==void 0&&e.set(n,o)}return e}function Oe(t){return t.endsWith("/")?t.substring(0,t.length-1):t}var ut=Object.defineProperty,lt=(t,e,r)=>e in t?ut(t,e,{enumerable:!0,configurable:!0,writable:!0,value:r}):t[e]=r,R=(t,e,r)=>lt(t,typeof e!="symbol"?e+"":e,r);const ct=(t,e)=>{if(typeof FormData<"u"&&t instanceof FormData||typeof Buffer<"u"&&typeof Buffer.isBuffer=="function"&&Buffer.isBuffer(t)||typeof File<"u"&&t instanceof File||typeof Blob<"u"&&t instanceof Blob||typeof ArrayBuffer<"u"&&t instanceof ArrayBuffer||typeof ArrayBuffer<"u"&&ArrayBuffer.isView&&ArrayBuffer.isView(t))return t;let r;return e&&(e instanceof Headers?r=e.get("Content-Type")??e.get("content-type")??void 0:typeof e=="object"&&(r=e["Content-Type"]??e["content-type"]),r==="application/x-www-form-urlencoded")?t&&typeof t=="object"&&!(t instanceof URLSearchParams)?new URLSearchParams(t).toString():String(t):JSON.stringify(t)};async function ve(t){const e=(t.headers.get("content-type")||"").includes("application/json")?await t.clone().json():await t.clone().text(),r=typeof e=="object"&&e!==null?e.error||e.message:typeof e=="string"?e:void 0;return Object.assign(new Error(r||`HTTP ${t.status} ${t.statusText}`),{status:t.status,statusText:t.statusText,body:e})}function ft(t,e,r,i=3e3){let n,o=r??void 0,s,a=null;const u=async()=>{n||(a||(a=(async()=>{const l=await e();if(!l.api_semver)throw new Error("Empty API version");n=l.api_semver})().finally(()=>{a=null})),await a)},c={async onRequest({request:l,schemaPath:f}){return o&&l.headers.set("Authorization",`Bearer ${o}`),f!=="/version"&&(await u(),n&&l.headers.set("X-API-Sem-Ver",n),s&&l.headers.set("X-API-Token",s)),l},async onResponse({request:l,response:f,options:h,schemaPath:g}){if(f.ok)return f;if(g==="/version")throw await ve(f);if(f.status!==405)throw await ve(f);n=void 0,await u(),n&&l.headers.set("X-API-Sem-Ver",n),o&&l.headers.set("Authorization",`Bearer ${o}`);const m=await(h.fetch??fetch)(l);if(m.ok)return m;throw await ve(m)}},d=We({baseUrl:t,bodySerializer:ct});return d.withTimeout=async(l,f=i)=>{if(f<=0)return await l();const h=new AbortController,g=setTimeout(()=>h.abort(),f);try{return await l(h.signal)}catch(m){throw m instanceof DOMException&&m.name==="AbortError"?new Error(`Request timed out after ${f}ms`):m}finally{clearTimeout(g)}},d.use(c),{client:d,setApiKey:l=>{s=l},setToken:l=>{o=l}}}async function dt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/version",{signal:n}),e?.timeout);if(i)throw i;return r}async function ht(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function pt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/system",{signal:n}),e?.timeout);if(i)throw i;return r}async function mt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/power",{signal:n}),e?.timeout);if(i)throw i;return r}async function yt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/device",{signal:n}),e?.timeout);if(i)throw i;return r}async function vt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/status/firmware",{signal:n}),e?.timeout);if(i)throw i;return r}async function gt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/transport",{signal:n}),e?.timeout);if(i)throw i;return r}class bt{async SystemVersionGet(e){const r=await dt(this.apiClient,e);return this.apiSemver=r.api_semver,r}async SystemStatusGet(e){return await ht(this.apiClient,e)}async SystemInfoGet(e){return await pt(this.apiClient,e)}async SystemStatusPowerGet(e){return await mt(this.apiClient,e)}async SystemStatusDeviceGet(e){return await yt(this.apiClient,e)}async SystemStatusFirmwareGet(e){return await vt(this.apiClient,e)}async SystemTransportGet(e){return await gt(this.apiClient,e)}}async function wt(t,e){const{file:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/update",{headers:{"Content-Type":"application/octet-stream"},body:r,signal:o}),e.timeout);if(n)throw n;return i}async function Et(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/update/check",{signal:n}),e?.timeout);if(i)throw i;return r}async function _t(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/update/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function At(t,e){const{version:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.GET("/update/changelog",{params:{query:{version:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Tt(t,e){const{version:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/update/install",{params:{query:{version:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Rt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/update/abort_download",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ot(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/update/autoupdate",{signal:n}),e?.timeout);if(i)throw i;return r}async function St(t,e){const{is_enabled:r,interval_start:i,interval_end:n}=e,{data:o,error:s}=await t.withTimeout(a=>t.POST("/update/autoupdate",{body:{is_enabled:r,interval_start:i,interval_end:n},signal:a}),e.timeout);if(s)throw s;return o}class Nt{async UpdateFromFile(e){return await wt(this.apiClient,e)}async UpdateCheck(e){return await Et(this.apiClient,e)}async UpdateStatusGet(e){return await _t(this.apiClient,e)}async UpdateChangelogGet(e){return await At(this.apiClient,e)}async UpdateInstall(e){return await Tt(this.apiClient,e)}async UpdateAbort(e){return await Rt(this.apiClient,e)}async UpdateAutoUpdateGet(e){return await Ot(this.apiClient,e)}async UpdateAutoUpdateSet(e){return await St(this.apiClient,e)}}async function Ct(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/time",{signal:n}),e?.timeout);if(i)throw i;return r}async function kt(t,e){const{timestamp:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/time/timestamp",{params:{query:{timestamp:r}},signal:o}),e.timeout);if(n)throw n;return i}async function It(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/time/timezone",{signal:n}),e?.timeout);if(i)throw i;return r}async function qt(t,e){const{timezone:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/time/timezone",{params:{query:{timezone:r}},signal:o}),e.timeout);if(n)throw n;return i}async function Pt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/time/tzlist",{signal:n}),e?.timeout);if(i)throw i;return r}class Dt{async TimeGet(e){return await Ct(this.apiClient,e)}async TimeTimestampSet(e){return await kt(this.apiClient,e)}async TimeTimezoneGet(e){return await It(this.apiClient,e)}async TimeTimezoneSet(e){return await qt(this.apiClient,e)}async TimeTzListGet(e){return await Pt(this.apiClient,e)}}async function xt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/account/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function jt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/account/info",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ut(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/account/profile",{signal:n}),e?.timeout);if(i)throw i;return r}async function Lt(t,e){const{profile:r,custom_url:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/account/profile",{params:{query:{profile:r,custom_url:i}},signal:s}),e.timeout);if(o)throw o;return n}async function Bt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/account",{signal:n}),e?.timeout);if(i)throw i;return r}async function Ft(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/account/link",{signal:n}),e?.timeout);if(i)throw i;return r}class Mt{async AccountInfoGet(e){return await jt(this.apiClient,e)}async AccountStateGet(e){return await xt(this.apiClient,e)}async AccountProfileGet(e){return await Ut(this.apiClient,e)}async AccountProfileSet(e){return await Lt(this.apiClient,e)}async AccountUnlink(e){return await Bt(this.apiClient,e)}async AccountLink(e){return await Ft(this.apiClient,e)}}async function Wt(t,e){const{application_name:r,elements:i,priority:n=50,timeout:o}=e,{data:s,error:a}=await t.withTimeout(u=>t.POST("/display/draw",{body:{application_name:r,priority:n,elements:i},signal:u}),o);if(a)throw a;return s}async function Gt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/display/draw",{params:{query:{application_name:e?.application_name}},signal:n}),e?.timeout);if(i)throw i;return r}async function $t(t,e){const{display:r,timeout:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.GET("/screen",{params:{query:{display:r}},parseAs:"blob",signal:s}),i);if(o)throw o;return n}async function Vt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/display/brightness",{signal:n}),e?.timeout);if(i)throw i;return r}async function zt(t,e){const{value:r}=e,i=(s=>{if(typeof s=="number"){if(s<0||s>100)throw new Error("Brightness value must be between 0 and 100 or 'auto'");return String(s)}return"auto"})(r),{data:n,error:o}=await t.withTimeout(s=>t.POST("/display/brightness",{params:{query:{value:i}},signal:s}),e.timeout);if(o)throw o;return n}class Ht{async DisplayDraw(e){return await Wt(this.apiClient,e)}async DisplayClear(e){return await Gt(this.apiClient,e)}async DisplayScreenFrameGet(e){return await $t(this.apiClient,e)}async DisplayBrightnessGet(e){return await Vt(this.apiClient,e)}async DisplayBrightnessSet(e){return await zt(this.apiClient,e)}}async function Jt(t,e){const{application_name:r,path:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/audio/play",{params:{query:{application_name:r,path:i}},signal:s}),e.timeout);if(o)throw o;return n}async function Kt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/audio/play",{signal:n}),e?.timeout);if(i)throw i;return r}async function Xt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/audio/volume",{signal:n}),e?.timeout);if(i)throw i;return r}async function Yt(t,e){const{volume:r,silent:i}=e;if(typeof r!="number"||r<0||r>100)throw new Error("Volume must be a number between 0 and 100");const{data:n,error:o}=await t.withTimeout(s=>t.POST("/audio/volume",{params:{query:{volume:r,silent:i}},signal:s}),e.timeout);if(o)throw o;return n}class Zt{async AudioPlay(e){return await Jt(this.apiClient,e)}async AudioStop(e){return await Kt(this.apiClient,e)}async AudioVolumeGet(e){return await Xt(this.apiClient,e)}async AudioVolumeSet(e){return await Yt(this.apiClient,e)}}async function Qt(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/wifi/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function er(t,e){const{ssid:r,password:i,security:n,ip_config:o}=e,{data:s,error:a}=await t.withTimeout(u=>t.POST("/wifi/connect",{body:{ssid:r,password:i,security:n,ip_config:o},signal:u}),e.timeout);if(a)throw a;return s}async function tr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/wifi/disconnect",{signal:n}),e?.timeout);if(i)throw i;return r}async function rr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/wifi/networks",{signal:n}),e?.timeout);if(i)throw i;return r}class ir{async WifiStatusGet(e){return await Qt(this.apiClient,e)}async WifiConnect(e){return await er(this.apiClient,e)}async WifiDisconnect(e){return await tr(this.apiClient,e)}async WifiNetworksGet(e){return await rr(this.apiClient,e)}}async function nr(t,e){const{path:r,file:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/storage/write",{params:{query:{path:r}},headers:{"Content-Type":"application/octet-stream"},body:i,signal:s}),e.timeout);if(o)throw o;return n}async function or(t,e){const{path:r,as_array_buffer:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.GET("/storage/read",{params:{query:{path:r}},parseAs:i?"arrayBuffer":"blob",signal:s}),e.timeout);if(o)throw o;return n}async function sr(t,e){const{path:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.GET("/storage/list",{params:{query:{path:r}},signal:o}),e.timeout);if(n)throw n;return i}async function ar(t,e){const{path:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.DELETE("/storage/remove",{params:{query:{path:r}},signal:o}),e.timeout);if(n)throw n;return i}async function ur(t,e){const{path:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/storage/mkdir",{params:{query:{path:r}},signal:o}),e.timeout);if(n)throw n;return i}async function lr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/storage/status",{signal:n}),e?.timeout);if(i)throw i;return r}async function cr(t,e){const{path:r,new_path:i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/storage/rename",{params:{query:{path:r,new_path:i}},signal:s}),e.timeout);if(o)throw o;return n}class fr{async StorageWrite(e){return await nr(this.apiClient,e)}async StorageRead(e){return await or(this.apiClient,e)}async StorageListGet(e){return await sr(this.apiClient,e)}async StorageRemove(e){return await ar(this.apiClient,e)}async StorageMkdir(e){return await ur(this.apiClient,e)}async StorageStatusGet(e){return await lr(this.apiClient,e)}async StorageRename(e){return await cr(this.apiClient,e)}}async function dr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/access",{signal:n}),e?.timeout);if(i)throw i;return r}async function hr(t,e){const{mode:r,key:i}=e,n=i??"";if(String(n).trim()&&!/^\d{4,10}$/.test(String(n)))throw new Error("Key must be a string of 4 to 10 digits");const{data:o,error:s}=await t.withTimeout(a=>t.POST("/access",{params:{query:{mode:r,key:n}},signal:a}),e.timeout);if(s)throw s;return o}async function pr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/name",{signal:n}),e?.timeout);if(i)throw i;return r}async function mr(t,e){const{name:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/name",{body:{name:r},signal:o}),e.timeout);if(n)throw n;return i}class yr{async SettingsAccessGet(e){return await dr(this.apiClient,e)}async SettingsAccessSet(e){const r=await hr(this.apiClient,e);return e.mode==="key"&&e.key&&this.setApiKey(e.key),r}async SettingsNameGet(e){return await pr(this.apiClient,e)}async SettingsNameSet(e){return await mr(this.apiClient,e)}}async function vr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/ble/enable",{signal:n}),e?.timeout);if(i)throw i;return r}async function gr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/ble/disable",{signal:n}),e?.timeout);if(i)throw i;return r}async function br(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/ble/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function wr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/ble/status",{signal:n}),e?.timeout);if(i)throw i;return r}class Er{async BleEnable(e){return await vr(this.apiClient,e)}async BleDisable(e){return await gr(this.apiClient,e)}async BleUnpair(e){return await br(this.apiClient,e)}async BleStatusGet(e){return await wr(this.apiClient,e)}}async function _r(t,e){const{key:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.POST("/input",{params:{query:{key:r}},signal:o}),e.timeout);if(n)throw n;return i}class Ar{async InputSend(e){return await _r(this.apiClient,e)}}async function Tr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/smart_home/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function Rr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.POST("/smart_home/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function Or(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.DELETE("/smart_home/pairing",{signal:n}),e?.timeout);if(i)throw i;return r}async function Sr(t,e){const{data:r,error:i}=await t.withTimeout(n=>t.GET("/smart_home/switch",{signal:n}),e?.timeout);if(i)throw i;return r}async function Nr(t,e){const{timeout:r,...i}=e,{data:n,error:o}=await t.withTimeout(s=>t.POST("/smart_home/switch",{body:i,signal:s}),r);if(o)throw o;return n}class Cr{async SmartHomePairingGet(e){return await Tr(this.apiClient,e)}async SmartHomePair(e){return await Rr(this.apiClient,e)}async SmartHomeErase(e){return await Or(this.apiClient,e)}async SmartHomeSwitchStateGet(e){return await Sr(this.apiClient,e)}async SmartHomeSwitchStateSet(e){return await Nr(this.apiClient,e)}async MatterStatusGet(e){return await this.SmartHomePairingGet(e)}async MatterPair(e){return await this.SmartHomePair(e)}async MatterErase(e){return await this.SmartHomeErase(e)}}async function kr(t,e){const{application_name:r,file:i,data:n}=e,{data:o,error:s}=await t.withTimeout(a=>t.POST("/assets/upload",{params:{query:{application_name:r,file:i}},headers:{"Content-Type":"application/octet-stream"},body:n,signal:a}),e.timeout);if(s)throw s;return o}async function Ir(t,e){const{application_name:r}=e,{data:i,error:n}=await t.withTimeout(o=>t.DELETE("/assets/upload",{params:{query:{application_name:r}},signal:o}),e.timeout);if(n)throw n;return i}class qr{async AssetsUpload(e){return await kr(this.apiClient,e)}async AssetsDelete(e){return await Ir(this.apiClient,e)}}const Pr="http://10.0.4.20",Dr="https://proxy.busy.app",xr=/^https?:\/\/proxy(?:\.(?:dev|test|stage))?\.busy\.app$/i;function jr(t){const e=t.split(".");if(e.length!==4)return!1;for(const r of e){if(r.length===0||r.length>1&&r[0]==="0"||!/^\d+$/.test(r))return!1;const i=Number(r);if(i<0||i>255)return!1}return!0}function Ur(t){return/\.local$/i.test(t)}class Ve{constructor(e){if(R(this,"addr"),R(this,"apiSemver"),R(this,"apiClient"),R(this,"setApiKeyFn"),R(this,"setTokenFn"),R(this,"connectionType","unknown"),!e||!e.addr&&!e.token)this.addr=Pr;else if(!e.addr)this.addr=Dr;else{let o=e.addr.trim();if(/^https?:\/\//i.test(o)||(o=`http://${o}`),xr.test(o)&&!e.token)throw new Error("Token is required. Please provide it.");this.addr=o}this.apiSemver="";const{client:r,setApiKey:i,setToken:n}=ft(`${this.addr}/api/`,this.SystemVersionGet.bind(this),e?.token,e?.timeout);this.apiClient=r,this.setApiKeyFn=i,this.setTokenFn=n,this.detectConnectionType()}async detectConnectionType(){const e=new URL(this.addr).hostname;if(!jr(e)&&!Ur(e)){this.connectionType="wifi";return}const r=We({baseUrl:`${this.addr}/api/`});try{const{response:i}=await r.GET("/name");if(i.status===401||i.status===403)this.connectionType="wifi";else if(i.ok)this.connectionType="usb";else throw new Error(`Failed to detect connection type. Status: ${i.status}`)}catch(i){throw i}}setApiKey(e){this.setApiKeyFn(e)}setToken(e){this.setTokenFn(e)}}function Lr(t,e){e.forEach(r=>{Object.getOwnPropertyNames(r.prototype).forEach(i=>{Object.defineProperty(t.prototype,i,Object.getOwnPropertyDescriptor(r.prototype,i)||Object.create(null))})})}Lr(Ve,[bt,Nt,Dt,Mt,Ht,Zt,ir,fr,yr,Er,Ar,Cr,qr]);var x=(t=>(t.CONNECTION_FAILED="CONNECTION_FAILED",t.RECONNECT_FAILED="RECONNECT_FAILED",t.CONNECTION_LOST="CONNECTION_LOST",t.CONNECTION_TIMEOUT="CONNECTION_TIMEOUT",t.AUTH_FAILED="AUTH_FAILED",t.AUTH_REFRESH_FAILED="AUTH_REFRESH_FAILED",t.DEVICE_ERROR="DEVICE_ERROR",t.DECODE_ERROR="DECODE_ERROR",t.FRAME_PROCESS_ERROR="FRAME_PROCESS_ERROR",t.STREAM_ALREADY_STARTED="STREAM_ALREADY_STARTED",t.WORKER_INIT_FAILED="WORKER_INIT_FAILED",t.UNKNOWN_ERROR="UNKNOWN_ERROR",t))(x||{});class Q extends Error{constructor(e,r,i){super(r),this.code=e,this.data=i,this.name="StateStreamError"}}var j=(t=>(t.IDLE="IDLE",t.STARTING="STARTING",t.RUNNING="RUNNING",t.STOPPED="STOPPED",t.FAILED="FAILED",t))(j||{}),ee=(t=>(t.DISCONNECTED="DISCONNECTED",t.CONNECTING="CONNECTING",t.CONNECTED="CONNECTED",t.RECONNECTING="RECONNECTING",t))(ee||{}),oe=(t=>(t.UNAUTHENTICATED="UNAUTHENTICATED",t.AUTHENTICATING="AUTHENTICATING",t.AUTHENTICATED="AUTHENTICATED",t.FAILED="FAILED",t))(oe||{}),Y=(t=>(t.NONE="NONE",t.ACTIVE="ACTIVE",t.STALE="STALE",t))(Y||{}),se=(t=>(t.OFF="OFF",t.INITIALIZING="INITIALIZING",t.READY="READY",t.ERROR="ERROR",t))(se||{});const ze=`var commonjsGlobal = typeof globalThis < "u" ? globalThis : typeof window < "u" ? window : typeof global < "u" ? global : typeof self < "u" ? self : {}, src = { exports: {} }, indexLight = { exports: {} }, indexMinimal = {}, minimal = {}, aspromise, hasRequiredAspromise;
function requireAspromise() {
  if (hasRequiredAspromise) return aspromise;
  hasRequiredAspromise = 1, aspromise = u;
  function u(f, h) {
    for (var c = new Array(arguments.length - 1), d = 0, n = 2, e = !0; n < arguments.length; )
      c[d++] = arguments[n++];
    return new Promise(function(i, t) {
      c[d] = function(s) {
        if (e)
          if (e = !1, s)
            t(s);
          else {
            for (var a = new Array(arguments.length - 1), o = 0; o < a.length; )
              a[o++] = arguments[o];
            i.apply(null, a);
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
  return hasRequiredBase64 || (hasRequiredBase64 = 1, function(u) {
    var f = u;
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
      for (var l = null, s = [], a = 0, o = 0, p; i < t; ) {
        var y = r[i++];
        switch (o) {
          case 0:
            s[a++] = h[y >> 2], p = (y & 3) << 4, o = 1;
            break;
          case 1:
            s[a++] = h[p | y >> 4], p = (y & 15) << 2, o = 2;
            break;
          case 2:
            s[a++] = h[p | y >> 6], s[a++] = h[y & 63], o = 0;
            break;
        }
        a > 8191 && ((l || (l = [])).push(String.fromCharCode.apply(String, s)), a = 0);
      }
      return o && (s[a++] = h[p], s[a++] = 61, o === 1 && (s[a++] = 61)), l ? (a && l.push(String.fromCharCode.apply(String, s.slice(0, a))), l.join("")) : String.fromCharCode.apply(String, s.slice(0, a));
    };
    var n = "invalid encoding";
    f.decode = function(r, i, t) {
      for (var l = t, s = 0, a, o = 0; o < r.length; ) {
        var p = r.charCodeAt(o++);
        if (p === 61 && s > 1)
          break;
        if ((p = c[p]) === void 0)
          throw Error(n);
        switch (s) {
          case 0:
            a = p, s = 1;
            break;
          case 1:
            i[t++] = a << 2 | (p & 48) >> 4, a = p, s = 2;
            break;
          case 2:
            i[t++] = (a & 15) << 4 | (p & 60) >> 2, a = p, s = 3;
            break;
          case 3:
            i[t++] = (a & 3) << 6 | p, s = 0;
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
  hasRequiredEventemitter = 1, eventemitter = u;
  function u() {
    this._listeners = {};
  }
  return u.prototype.on = function(h, c, d) {
    return (this._listeners[h] || (this._listeners[h] = [])).push({
      fn: c,
      ctx: d || this
    }), this;
  }, u.prototype.off = function(h, c) {
    if (h === void 0)
      this._listeners = {};
    else if (c === void 0)
      this._listeners[h] = [];
    else
      for (var d = this._listeners[h], n = 0; n < d.length; )
        d[n].fn === c ? d.splice(n, 1) : ++n;
    return this;
  }, u.prototype.emit = function(h) {
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
  hasRequiredFloat = 1, float = u(u);
  function u(n) {
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
      function a(o, p) {
        return r[3] = o[p], r[2] = o[p + 1], r[1] = o[p + 2], r[0] = o[p + 3], e[0];
      }
      n.readFloatLE = i ? s : a, n.readFloatBE = i ? a : s;
    }() : function() {
      function e(i, t, l, s) {
        var a = t < 0 ? 1 : 0;
        if (a && (t = -t), t === 0)
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
          i((a << 31 | 2139095040) >>> 0, l, s);
        else if (t < 11754943508222875e-54)
          i((a << 31 | Math.round(t / 1401298464324817e-60)) >>> 0, l, s);
        else {
          var o = Math.floor(Math.log(t) / Math.LN2), p = Math.round(t * Math.pow(2, -o) * 8388608) & 8388607;
          i((a << 31 | o + 127 << 23 | p) >>> 0, l, s);
        }
      }
      n.writeFloatLE = e.bind(null, f), n.writeFloatBE = e.bind(null, h);
      function r(i, t, l) {
        var s = i(t, l), a = (s >> 31) * 2 + 1, o = s >>> 23 & 255, p = s & 8388607;
        return o === 255 ? p ? NaN : a * (1 / 0) : o === 0 ? a * 1401298464324817e-60 * p : a * Math.pow(2, o - 150) * (p + 8388608);
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
      function a(o, p) {
        return r[7] = o[p], r[6] = o[p + 1], r[5] = o[p + 2], r[4] = o[p + 3], r[3] = o[p + 4], r[2] = o[p + 5], r[1] = o[p + 6], r[0] = o[p + 7], e[0];
      }
      n.readDoubleLE = i ? s : a, n.readDoubleBE = i ? a : s;
    }() : function() {
      function e(i, t, l, s, a, o) {
        var p = s < 0 ? 1 : 0;
        if (p && (s = -s), s === 0)
          i(0, a, o + t), i(1 / s > 0 ? (
            /* positive */
            0
          ) : (
            /* negative 0 */
            2147483648
          ), a, o + l);
        else if (isNaN(s))
          i(0, a, o + t), i(2146959360, a, o + l);
        else if (s > 17976931348623157e292)
          i(0, a, o + t), i((p << 31 | 2146435072) >>> 0, a, o + l);
        else {
          var y;
          if (s < 22250738585072014e-324)
            y = s / 5e-324, i(y >>> 0, a, o + t), i((p << 31 | y / 4294967296) >>> 0, a, o + l);
          else {
            var E = Math.floor(Math.log(s) / Math.LN2);
            E === 1024 && (E = 1023), y = s * Math.pow(2, -E), i(y * 4503599627370496 >>> 0, a, o + t), i((p << 31 | E + 1023 << 20 | y * 1048576 & 1048575) >>> 0, a, o + l);
          }
        }
      }
      n.writeDoubleLE = e.bind(null, f, 0, 4), n.writeDoubleBE = e.bind(null, h, 4, 0);
      function r(i, t, l, s, a) {
        var o = i(s, a + t), p = i(s, a + l), y = (p >> 31) * 2 + 1, E = p >>> 20 & 2047, v = 4294967296 * (p & 1048575) + o;
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
    } catch (u) {
    }
    return null;
  }
  return inquire_1;
}
var utf8 = {}, hasRequiredUtf8;
function requireUtf8() {
  return hasRequiredUtf8 || (hasRequiredUtf8 = 1, function(u) {
    var f = u;
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
  hasRequiredPool = 1, pool_1 = u;
  function u(f, h, c) {
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
  var u = requireMinimal();
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
    if (u.isString(e))
      if (u.Long)
        e = u.Long.fromString(e);
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
    return u.Long ? new u.Long(this.lo | 0, this.hi | 0, !!e) : { low: this.lo | 0, high: this.hi | 0, unsigned: !!e };
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
  return hasRequiredMinimal || (hasRequiredMinimal = 1, function(u) {
    var f = u;
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
  var u = requireMinimal(), f, h = u.LongBits, c = u.base64, d = u.utf8;
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
    return u.Buffer ? function() {
      return (i.create = function() {
        return new f();
      })();
    } : function() {
      return new i();
    };
  };
  i.create = t(), i.alloc = function(v) {
    return new u.Array(v);
  }, u.Array !== Array && (i.alloc = u.pool(i.alloc, u.Array.prototype.subarray)), i.prototype._push = function(v, m, _) {
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
  function a(E, v) {
    this.len = E, this.next = void 0, this.val = v;
  }
  a.prototype = Object.create(n.prototype), a.prototype.fn = s, i.prototype.uint32 = function(v) {
    return this.len += (this.tail = this.tail.next = new a(
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
    return this._push(u.float.writeFloatLE, 4, v);
  }, i.prototype.double = function(v) {
    return this._push(u.float.writeDoubleLE, 8, v);
  };
  var y = u.Array.prototype.set ? function(v, m, _) {
    m.set(v, _);
  } : function(v, m, _) {
    for (var b = 0; b < v.length; ++b)
      m[_ + b] = v[b];
  };
  return i.prototype.bytes = function(v) {
    var m = v.length >>> 0;
    if (!m)
      return this._push(l, 1, 0);
    if (u.isString(v)) {
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
  var u = requireWriter();
  (h.prototype = Object.create(u.prototype)).constructor = h;
  var f = requireMinimal();
  function h() {
    u.call(this);
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
  var u = requireMinimal(), f, h = u.LongBits, c = u.utf8;
  function d(s, a) {
    return RangeError("index out of range: " + s.pos + " + " + (a || 1) + " > " + s.len);
  }
  function n(s) {
    this.buf = s, this.pos = 0, this.len = s.length;
  }
  var e = typeof Uint8Array < "u" ? function(a) {
    if (a instanceof Uint8Array || Array.isArray(a))
      return new n(a);
    throw Error("illegal buffer");
  } : function(a) {
    if (Array.isArray(a))
      return new n(a);
    throw Error("illegal buffer");
  }, r = function() {
    return u.Buffer ? function(o) {
      return (n.create = function(y) {
        return u.Buffer.isBuffer(y) ? new f(y) : e(y);
      })(o);
    } : e;
  };
  n.create = r(), n.prototype._slice = u.Array.prototype.subarray || /* istanbul ignore next */
  u.Array.prototype.slice, n.prototype.uint32 = /* @__PURE__ */ function() {
    var a = 4294967295;
    return function() {
      if (a = (this.buf[this.pos] & 127) >>> 0, this.buf[this.pos++] < 128 || (a = (a | (this.buf[this.pos] & 127) << 7) >>> 0, this.buf[this.pos++] < 128) || (a = (a | (this.buf[this.pos] & 127) << 14) >>> 0, this.buf[this.pos++] < 128) || (a = (a | (this.buf[this.pos] & 127) << 21) >>> 0, this.buf[this.pos++] < 128) || (a = (a | (this.buf[this.pos] & 15) << 28) >>> 0, this.buf[this.pos++] < 128)) return a;
      if ((this.pos += 5) > this.len)
        throw this.pos = this.len, d(this, 10);
      return a;
    };
  }(), n.prototype.int32 = function() {
    return this.uint32() | 0;
  }, n.prototype.sint32 = function() {
    var a = this.uint32();
    return a >>> 1 ^ -(a & 1) | 0;
  };
  function i() {
    var s = new h(0, 0), a = 0;
    if (this.len - this.pos > 4) {
      for (; a < 4; ++a)
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << a * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      if (s.lo = (s.lo | (this.buf[this.pos] & 127) << 28) >>> 0, s.hi = (s.hi | (this.buf[this.pos] & 127) >> 4) >>> 0, this.buf[this.pos++] < 128)
        return s;
      a = 0;
    } else {
      for (; a < 3; ++a) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << a * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
      return s.lo = (s.lo | (this.buf[this.pos++] & 127) << a * 7) >>> 0, s;
    }
    if (this.len - this.pos > 4) {
      for (; a < 5; ++a)
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << a * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
    } else
      for (; a < 5; ++a) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << a * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
    throw Error("invalid varint encoding");
  }
  n.prototype.bool = function() {
    return this.uint32() !== 0;
  };
  function t(s, a) {
    return (s[a - 4] | s[a - 3] << 8 | s[a - 2] << 16 | s[a - 1] << 24) >>> 0;
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
    var a = u.float.readFloatLE(this.buf, this.pos);
    return this.pos += 4, a;
  }, n.prototype.double = function() {
    if (this.pos + 8 > this.len)
      throw d(this, 4);
    var a = u.float.readDoubleLE(this.buf, this.pos);
    return this.pos += 8, a;
  }, n.prototype.bytes = function() {
    var a = this.uint32(), o = this.pos, p = this.pos + a;
    if (p > this.len)
      throw d(this, a);
    if (this.pos += a, Array.isArray(this.buf))
      return this.buf.slice(o, p);
    if (o === p) {
      var y = u.Buffer;
      return y ? y.alloc(0) : new this.buf.constructor(0);
    }
    return this._slice.call(this.buf, o, p);
  }, n.prototype.string = function() {
    var a = this.bytes();
    return c.read(a, 0, a.length);
  }, n.prototype.skip = function(a) {
    if (typeof a == "number") {
      if (this.pos + a > this.len)
        throw d(this, a);
      this.pos += a;
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
    var a = u.Long ? "toLong" : (
      /* istanbul ignore next */
      "toNumber"
    );
    u.merge(n.prototype, {
      int64: function() {
        return i.call(this)[a](!1);
      },
      uint64: function() {
        return i.call(this)[a](!0);
      },
      sint64: function() {
        return i.call(this).zzDecode()[a](!1);
      },
      fixed64: function() {
        return l.call(this)[a](!0);
      },
      sfixed64: function() {
        return l.call(this)[a](!1);
      }
    });
  }, reader;
}
var reader_buffer, hasRequiredReader_buffer;
function requireReader_buffer() {
  if (hasRequiredReader_buffer) return reader_buffer;
  hasRequiredReader_buffer = 1, reader_buffer = h;
  var u = requireReader();
  (h.prototype = Object.create(u.prototype)).constructor = h;
  var f = requireMinimal();
  function h(c) {
    u.call(this, c);
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
  var u = requireMinimal();
  (f.prototype = Object.create(u.EventEmitter.prototype)).constructor = f;
  function f(h, c, d) {
    if (typeof h != "function")
      throw TypeError("rpcImpl must be a function");
    u.EventEmitter.call(this), this.rpcImpl = h, this.requestDelimited = !!c, this.responseDelimited = !!d;
  }
  return f.prototype.rpcCall = function h(c, d, n, e, r) {
    if (!e)
      throw TypeError("request must be specified");
    var i = this;
    if (!r)
      return u.asPromise(h, i, c, d, n, e);
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
            } catch (a) {
              return i.emit("error", a, c), r(a);
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
  return hasRequiredRpc || (hasRequiredRpc = 1, function(u) {
    var f = u;
    f.Service = requireService$1();
  }(rpc)), rpc;
}
var roots, hasRequiredRoots;
function requireRoots() {
  return hasRequiredRoots || (hasRequiredRoots = 1, roots = {}), roots;
}
var hasRequiredIndexMinimal;
function requireIndexMinimal() {
  return hasRequiredIndexMinimal || (hasRequiredIndexMinimal = 1, function(u) {
    var f = u;
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
  hasRequiredCodegen = 1, codegen_1 = u;
  function u(f, h) {
    typeof f == "string" && (h = f, f = void 0);
    var c = [];
    function d(e) {
      if (typeof e != "string") {
        var r = n();
        if (u.verbose && console.log("codegen: " + r), r = "return " + r, e) {
          for (var i = Object.keys(e), t = new Array(i.length + 1), l = new Array(i.length), s = 0; s < i.length; )
            t[s] = i[s], l[s] = e[i[s++]];
          return t[s] = r, Function.apply(null, t).apply(null, l);
        }
        return Function(r)();
      }
      for (var a = new Array(arguments.length - 1), o = 0; o < a.length; )
        a[o] = arguments[++o];
      if (o = 0, e = e.replace(/%([%dfijs])/g, function(y, E) {
        var v = a[o++];
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
      }), o !== a.length)
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
  return u.verbose = !1, codegen_1;
}
var fetch_1, hasRequiredFetch;
function requireFetch() {
  if (hasRequiredFetch) return fetch_1;
  hasRequiredFetch = 1, fetch_1 = c;
  var u = requireAspromise(), f = requireInquire(), h = f("fs");
  function c(d, n, e) {
    return typeof n == "function" ? (e = n, n = {}) : n || (n = {}), e ? !n.xhr && h && h.readFile ? h.readFile(d, function(i, t) {
      return i && typeof XMLHttpRequest < "u" ? c.xhr(d, n, e) : i ? e(i) : e(null, n.binary ? t : t.toString("utf8"));
    }) : c.xhr(d, n, e) : u(c, this, d, n);
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
  return hasRequiredPath || (hasRequiredPath = 1, function(u) {
    var f = u, h = (
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
  var u = requireObject();
  ((i.prototype = Object.create(u.prototype)).constructor = i).className = "Namespace";
  var f = requireField(), h = requireUtil(), c = requireOneof(), d, n, e;
  i.fromJSON = function(s, a) {
    return new i(s, a.options).addJSON(a.nested);
  };
  function r(l, s) {
    if (l && l.length) {
      for (var a = {}, o = 0; o < l.length; ++o)
        a[l[o].name] = l[o].toJSON(s);
      return a;
    }
  }
  i.arrayToJSON = r, i.isReservedId = function(s, a) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (typeof s[o] != "string" && s[o][0] <= a && s[o][1] > a)
          return !0;
    }
    return !1;
  }, i.isReservedName = function(s, a) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (s[o] === a)
          return !0;
    }
    return !1;
  };
  function i(l, s) {
    u.call(this, l, s), this.nested = void 0, this._nestedArray = null, this._lookupCache = {}, this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
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
    var a = this;
    if (s)
      for (var o = Object.keys(s), p = 0, y; p < o.length; ++p)
        y = s[o[p]], a.add(
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
      var a = this.get(s.name);
      if (a)
        if (a instanceof i && s instanceof i && !(a instanceof d || a instanceof n)) {
          for (var o = a.nestedArray, p = 0; p < o.length; ++p)
            s.add(o[p]);
          this.remove(a), this.nested || (this.nested = {}), s.setOptions(a.options, !0);
        } else
          throw Error("duplicate name '" + s.name + "' in " + this);
    }
    this.nested[s.name] = s, this instanceof d || this instanceof n || this instanceof e || this instanceof f || s._edition || (s._edition = s._defaultEdition), this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
    for (var y = this; y = y.parent; )
      y._needsRecursiveFeatureResolution = !0, y._needsRecursiveResolve = !0;
    return s.onAdd(this), t(this);
  }, i.prototype.remove = function(s) {
    if (!(s instanceof u))
      throw TypeError("object must be a ReflectionObject");
    if (s.parent !== this)
      throw Error(s + " is not a member of " + this);
    return delete this.nested[s.name], Object.keys(this.nested).length || (this.nested = void 0), s.onRemove(this), t(this);
  }, i.prototype.define = function(s, a) {
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
    return a && o.addJSON(a), o;
  }, i.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    this._resolveFeaturesRecursive(this._edition);
    var s = this.nestedArray, a = 0;
    for (this.resolve(); a < s.length; )
      s[a] instanceof i ? s[a++].resolveAll() : s[a++].resolve();
    return this._needsRecursiveResolve = !1, this;
  }, i.prototype._resolveFeaturesRecursive = function(s) {
    return this._needsRecursiveFeatureResolution ? (this._needsRecursiveFeatureResolution = !1, s = this._edition || s, u.prototype._resolveFeaturesRecursive.call(this, s), this.nestedArray.forEach((a) => {
      a._resolveFeaturesRecursive(s);
    }), this) : this;
  }, i.prototype.lookup = function(s, a, o) {
    if (typeof a == "boolean" ? (o = a, a = void 0) : a && !Array.isArray(a) && (a = [a]), h.isString(s) && s.length) {
      if (s === ".")
        return this.root;
      s = s.split(".");
    } else if (!s.length)
      return this;
    var p = s.join(".");
    if (s[0] === "")
      return this.root.lookup(s.slice(1), a);
    var y = this.root._fullyQualifiedObjects && this.root._fullyQualifiedObjects["." + p];
    if (y && (!a || a.indexOf(y.constructor) > -1) || (y = this._lookupImpl(s, p), y && (!a || a.indexOf(y.constructor) > -1)))
      return y;
    if (o)
      return null;
    for (var E = this; E.parent; ) {
      if (y = E.parent._lookupImpl(s, p), y && (!a || a.indexOf(y.constructor) > -1))
        return y;
      E = E.parent;
    }
    return null;
  }, i.prototype._lookupImpl = function(s, a) {
    if (Object.prototype.hasOwnProperty.call(this._lookupCache, a))
      return this._lookupCache[a];
    var o = this.get(s[0]), p = null;
    if (o)
      s.length === 1 ? p = o : o instanceof i && (s = s.slice(1), p = o._lookupImpl(s, s.join(".")));
    else
      for (var y = 0; y < this.nestedArray.length; ++y)
        this._nestedArray[y] instanceof i && (o = this._nestedArray[y]._lookupImpl(s, a)) && (p = o);
    return this._lookupCache[a] = p, p;
  }, i.prototype.lookupType = function(s) {
    var a = this.lookup(s, [d]);
    if (!a)
      throw Error("no such type: " + s);
    return a;
  }, i.prototype.lookupEnum = function(s) {
    var a = this.lookup(s, [e]);
    if (!a)
      throw Error("no such Enum '" + s + "' in " + this);
    return a;
  }, i.prototype.lookupTypeOrEnum = function(s) {
    var a = this.lookup(s, [d, e]);
    if (!a)
      throw Error("no such Type or Enum '" + s + "' in " + this);
    return a;
  }, i.prototype.lookupService = function(s) {
    var a = this.lookup(s, [n]);
    if (!a)
      throw Error("no such Service '" + s + "' in " + this);
    return a;
  }, i._configure = function(l, s, a) {
    d = l, n = s, e = a;
  }, namespace;
}
var mapfield, hasRequiredMapfield;
function requireMapfield() {
  if (hasRequiredMapfield) return mapfield;
  hasRequiredMapfield = 1, mapfield = c;
  var u = requireField();
  ((c.prototype = Object.create(u.prototype)).constructor = c).className = "MapField";
  var f = requireTypes(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (u.call(this, d, n, r, void 0, void 0, i, t), !h.isString(e))
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
    return u.prototype.resolve.call(this);
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
  var u = requireObject();
  ((h.prototype = Object.create(u.prototype)).constructor = h).className = "Method";
  var f = requireUtil();
  function h(c, d, n, e, r, i, t, l, s) {
    if (f.isObject(r) ? (t = r, r = i = void 0) : f.isObject(i) && (t = i, i = void 0), !(d === void 0 || f.isString(d)))
      throw TypeError("type must be a string");
    if (!f.isString(n))
      throw TypeError("requestType must be a string");
    if (!f.isString(e))
      throw TypeError("responseType must be a string");
    u.call(this, c, t), this.type = d || "rpc", this.requestType = n, this.requestStream = r ? !0 : void 0, this.responseType = e, this.responseStream = i ? !0 : void 0, this.resolvedRequestType = null, this.resolvedResponseType = null, this.comment = l, this.parsedOptions = s;
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
    return this.resolved ? this : (this.resolvedRequestType = this.parent.lookupType(this.requestType), this.resolvedResponseType = this.parent.lookupType(this.responseType), u.prototype.resolve.call(this));
  }, method;
}
var service, hasRequiredService;
function requireService() {
  if (hasRequiredService) return service;
  hasRequiredService = 1, service = d;
  var u = requireNamespace();
  ((d.prototype = Object.create(u.prototype)).constructor = d).className = "Service";
  var f = requireMethod(), h = requireUtil(), c = requireRpc();
  function d(e, r) {
    u.call(this, e, r), this.methods = {}, this._methodsArray = null;
  }
  d.fromJSON = function(r, i) {
    var t = new d(r, i.options);
    if (i.methods)
      for (var l = Object.keys(i.methods), s = 0; s < l.length; ++s)
        t.add(f.fromJSON(l[s], i.methods[l[s]]));
    return i.nested && t.addJSON(i.nested), i.edition && (t._edition = i.edition), t.comment = i.comment, t._defaultEdition = "proto3", t;
  }, d.prototype.toJSON = function(r) {
    var i = u.prototype.toJSON.call(this, r), t = r ? !!r.keepComments : !1;
    return h.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      i && i.options || void 0,
      "methods",
      u.arrayToJSON(this.methodsArray, r) || /* istanbul ignore next */
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
    return this.methods[r] || u.prototype.get.call(this, r);
  }, d.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    u.prototype.resolve.call(this);
    for (var r = this.methodsArray, i = 0; i < r.length; ++i)
      r[i].resolve();
    return this;
  }, d.prototype._resolveFeaturesRecursive = function(r) {
    return this._needsRecursiveFeatureResolution ? (r = this._edition || r, u.prototype._resolveFeaturesRecursive.call(this, r), this.methodsArray.forEach((i) => {
      i._resolveFeaturesRecursive(r);
    }), this) : this;
  }, d.prototype.add = function(r) {
    if (this.get(r.name))
      throw Error("duplicate name '" + r.name + "' in " + this);
    return r instanceof f ? (this.methods[r.name] = r, r.parent = this, n(this)) : u.prototype.add.call(this, r);
  }, d.prototype.remove = function(r) {
    if (r instanceof f) {
      if (this.methods[r.name] !== r)
        throw Error(r + " is not a member of " + this);
      return delete this.methods[r.name], r.parent = null, n(this);
    }
    return u.prototype.remove.call(this, r);
  }, d.prototype.create = function(r, i, t) {
    for (var l = new c.Service(r, i, t), s = 0, a; s < /* initializes */
    this.methodsArray.length; ++s) {
      var o = h.lcFirst((a = this._methodsArray[s]).resolve().name).replace(/[^$\\w_]/g, "");
      l[o] = h.codegen(["r", "c"], h.isReserved(o) ? o + "_" : o)("return this.rpcCall(m,q,s,r,c)")({
        m: a,
        q: a.resolvedRequestType.ctor,
        s: a.resolvedResponseType.ctor
      });
    }
    return l;
  }, service;
}
var message, hasRequiredMessage;
function requireMessage() {
  if (hasRequiredMessage) return message;
  hasRequiredMessage = 1, message = f;
  var u = requireMinimal();
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
    return this.$type.toObject(this, u.toJSONOptions);
  }, message;
}
var decoder_1, hasRequiredDecoder;
function requireDecoder() {
  if (hasRequiredDecoder) return decoder_1;
  hasRequiredDecoder = 1, decoder_1 = d;
  var u = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n) {
    return "missing required '" + n.name + "'";
  }
  function d(n) {
    for (var e = h.codegen(["r", "l", "e"], n.name + "$decode")("if(!(r instanceof Reader))")("r=Reader.create(r)")("var c=l===undefined?r.len:r.pos+l,m=new this.ctor" + (n.fieldsArray.filter(function(a) {
      return a.map;
    }).length ? ",k,value" : ""))("while(r.pos<c){")("var t=r.uint32()")("if(t===e)")("break")("switch(t>>>3){"), r = 0; r < /* initializes */
    n.fieldsArray.length; ++r) {
      var i = n._fieldsArray[r].resolve(), t = i.resolvedType instanceof u ? "int32" : i.type, l = "m" + h.safeProp(i.name);
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
  var u = require_enum(), f = requireUtil();
  function h(e, r) {
    return e.name + ": " + r + (e.repeated && r !== "array" ? "[]" : e.map && r !== "object" ? "{k:" + e.keyType + "}" : "") + " expected";
  }
  function c(e, r, i, t) {
    if (r.resolvedType)
      if (r.resolvedType instanceof u) {
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
      var s = e._fieldsArray[l].resolve(), a = "m" + f.safeProp(s.name);
      if (s.optional && r("if(%s!=null&&m.hasOwnProperty(%j)){", a, s.name), s.map)
        r("if(!util.isObject(%s))", a)("return%j", h(s, "object"))("var k=Object.keys(%s)", a)("for(var i=0;i<k.length;++i){"), d(r, s, "k[i]"), c(r, s, l, a + "[k[i]]")("}");
      else if (s.repeated)
        r("if(!Array.isArray(%s))", a)("return%j", h(s, "array"))("for(var i=0;i<%s.length;++i){", a), c(r, s, l, a + "[i]")("}");
      else {
        if (s.partOf) {
          var o = f.safeProp(s.partOf.name);
          t[s.partOf.name] === 1 && r("if(p%s===1)", o)("return%j", s.partOf.name + ": multiple values"), t[s.partOf.name] = 1, r("p%s=1", o);
        }
        c(r, s, l, a);
      }
      s.optional && r("}");
    }
    return r("return null");
  }
  return verifier_1;
}
var converter = {}, hasRequiredConverter;
function requireConverter() {
  return hasRequiredConverter || (hasRequiredConverter = 1, function(u) {
    var f = u, h = require_enum(), c = requireUtil();
    function d(e, r, i, t) {
      var l = !1;
      if (r.resolvedType)
        if (r.resolvedType instanceof h) {
          e("switch(d%s){", t);
          for (var s = r.resolvedType.values, a = Object.keys(s), o = 0; o < a.length; ++o)
            s[a[o]] === r.typeDefault && !l && (e("default:")('if(typeof(d%s)==="number"){m%s=d%s;break}', t, t, t), r.repeated || e("break"), l = !0), e("case%j:", a[o])("case %i:", s[a[o]])("m%s=%j", t, s[a[o]])("break");
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
        var s = i[l].resolve(), a = c.safeProp(s.name);
        s.map ? (t("if(d%s){", a)('if(typeof d%s!=="object")', a)("throw TypeError(%j)", s.fullName + ": object expected")("m%s={}", a)("for(var ks=Object.keys(d%s),i=0;i<ks.length;++i){", a), d(
          t,
          s,
          /* not sorted */
          l,
          a + "[ks[i]]"
        )("}")("}")) : s.repeated ? (t("if(d%s){", a)("if(!Array.isArray(d%s))", a)("throw TypeError(%j)", s.fullName + ": array expected")("m%s=[]", a)("for(var i=0;i<d%s.length;++i){", a), d(
          t,
          s,
          /* not sorted */
          l,
          a + "[i]"
        )("}")("}")) : (s.resolvedType instanceof h || t("if(d%s!=null){", a), d(
          t,
          s,
          /* not sorted */
          l,
          a
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
      for (var t = c.codegen(["m", "o"], r.name + "$toObject")("if(!o)")("o={}")("var d={}"), l = [], s = [], a = [], o = 0; o < i.length; ++o)
        i[o].partOf || (i[o].resolve().repeated ? l : i[o].map ? s : a).push(i[o]);
      if (l.length) {
        for (t("if(o.arrays||o.defaults){"), o = 0; o < l.length; ++o) t("d%s=[]", c.safeProp(l[o].name));
        t("}");
      }
      if (s.length) {
        for (t("if(o.objects||o.defaults){"), o = 0; o < s.length; ++o) t("d%s={}", c.safeProp(s[o].name));
        t("}");
      }
      if (a.length) {
        for (t("if(o.defaults){"), o = 0; o < a.length; ++o) {
          var p = a[o], y = c.safeProp(p.name);
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
  return hasRequiredWrappers || (hasRequiredWrappers = 1, function(u) {
    var f = u, h = requireMessage();
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
  var u = requireNamespace();
  ((y.prototype = Object.create(u.prototype)).constructor = y).className = "Type";
  var f = require_enum(), h = requireOneof(), c = requireField(), d = requireMapfield(), n = requireService(), e = requireMessage(), r = requireReader(), i = requireWriter(), t = requireUtil(), l = requireEncoder(), s = requireDecoder(), a = requireVerifier(), o = requireConverter(), p = requireWrappers();
  function y(v, m) {
    v = v.replace(/\\W/g, ""), u.call(this, v, m), this.fields = {}, this.oneofs = void 0, this.extensions = void 0, this.reserved = void 0, this.group = void 0, this._fieldsById = null, this._fieldsArray = null, this._oneofsArray = null, this._ctor = null;
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
          var _ = this.fields[v[m]], b = _.id;
          if (this._fieldsById[b])
            throw Error("duplicate id " + b + " in " + this);
          this._fieldsById[b] = _;
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
        var b = {};
        for (_ = 0; _ < /* initializes */
        this.oneofsArray.length; ++_)
          b[this._oneofsArray[_].resolve().name] = {
            get: t.oneOfGetter(this._oneofsArray[_].oneof),
            set: t.oneOfSetter(this._oneofsArray[_].oneof)
          };
        _ && Object.defineProperties(v.prototype, b);
      }
    }
  }), y.generateConstructor = function(m) {
    for (var _ = t.codegen(["p"], m.name), b = 0, I; b < m.fieldsArray.length; ++b)
      (I = m._fieldsArray[b]).map ? _("this%s={}", t.safeProp(I.name)) : I.repeated && _("this%s=[]", t.safeProp(I.name));
    return _("if(p)for(var ks=Object.keys(p),i=0;i<ks.length;++i)if(p[ks[i]]!=null)")("this[ks[i]]=p[ks[i]]");
  };
  function E(v) {
    return v._fieldsById = v._fieldsArray = v._oneofsArray = null, delete v.encode, delete v.decode, delete v.verify, v;
  }
  return y.fromJSON = function(m, _) {
    var b = new y(m, _.options);
    b.extensions = _.extensions, b.reserved = _.reserved;
    for (var I = Object.keys(_.fields), C = 0; C < I.length; ++C)
      b.add(
        (typeof _.fields[I[C]].keyType < "u" ? d.fromJSON : c.fromJSON)(I[C], _.fields[I[C]])
      );
    if (_.oneofs)
      for (I = Object.keys(_.oneofs), C = 0; C < I.length; ++C)
        b.add(h.fromJSON(I[C], _.oneofs[I[C]]));
    if (_.nested)
      for (I = Object.keys(_.nested), C = 0; C < I.length; ++C) {
        var j = _.nested[I[C]];
        b.add(
          // most to least likely
          (j.id !== void 0 ? c.fromJSON : j.fields !== void 0 ? y.fromJSON : j.values !== void 0 ? f.fromJSON : j.methods !== void 0 ? n.fromJSON : u.fromJSON)(I[C], j)
        );
      }
    return _.extensions && _.extensions.length && (b.extensions = _.extensions), _.reserved && _.reserved.length && (b.reserved = _.reserved), _.group && (b.group = !0), _.comment && (b.comment = _.comment), _.edition && (b._edition = _.edition), b._defaultEdition = "proto3", b;
  }, y.prototype.toJSON = function(m) {
    var _ = u.prototype.toJSON.call(this, m), b = m ? !!m.keepComments : !1;
    return t.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      _ && _.options || void 0,
      "oneofs",
      u.arrayToJSON(this.oneofsArray, m),
      "fields",
      u.arrayToJSON(this.fieldsArray.filter(function(I) {
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
      b ? this.comment : void 0
    ]);
  }, y.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    u.prototype.resolveAll.call(this);
    var m = this.oneofsArray;
    for (b = 0; b < m.length; )
      m[b++].resolve();
    for (var _ = this.fieldsArray, b = 0; b < _.length; )
      _[b++].resolve();
    return this;
  }, y.prototype._resolveFeaturesRecursive = function(m) {
    return this._needsRecursiveFeatureResolution ? (m = this._edition || m, u.prototype._resolveFeaturesRecursive.call(this, m), this.oneofsArray.forEach((_) => {
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
    return m instanceof h ? (this.oneofs || (this.oneofs = {}), this.oneofs[m.name] = m, m.onAdd(this), E(this)) : u.prototype.add.call(this, m);
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
    return u.prototype.remove.call(this, m);
  }, y.prototype.isReservedId = function(m) {
    return u.isReservedId(this.reserved, m);
  }, y.prototype.isReservedName = function(m) {
    return u.isReservedName(this.reserved, m);
  }, y.prototype.create = function(m) {
    return new this.ctor(m);
  }, y.prototype.setup = function() {
    for (var m = this.fullName, _ = [], b = 0; b < /* initializes */
    this.fieldsArray.length; ++b)
      _.push(this._fieldsArray[b].resolve().resolvedType);
    this.encode = l(this)({
      Writer: i,
      types: _,
      util: t
    }), this.decode = s(this)({
      Reader: r,
      types: _,
      util: t
    }), this.verify = a(this)({
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
    return function(b) {
      t.decorateType(b, m);
    };
  }, type;
}
var root$1, hasRequiredRoot;
function requireRoot() {
  if (hasRequiredRoot) return root$1;
  hasRequiredRoot = 1, root$1 = i;
  var u = requireNamespace();
  ((i.prototype = Object.create(u.prototype)).constructor = i).className = "Root";
  var f = requireField(), h = require_enum(), c = requireOneof(), d = requireUtil(), n, e, r;
  function i(a) {
    u.call(this, "", a), this.deferred = [], this.files = [], this._edition = "proto2", this._fullyQualifiedObjects = {};
  }
  i.fromJSON = function(o, p) {
    return p || (p = new i()), o.options && p.setOptions(o.options), p.addJSON(o.nested).resolveAll();
  }, i.prototype.resolvePath = d.path.resolve, i.prototype.fetch = d.fetch;
  function t() {
  }
  i.prototype.load = function a(o, p, y) {
    typeof p == "function" && (y = p, p = void 0);
    var E = this;
    if (!y)
      return d.asPromise(a, E, o, p);
    var v = y === t;
    function m(D, P) {
      if (y) {
        if (v)
          throw D;
        P && P.resolveAll();
        var S = y;
        y = null, S(D, P);
      }
    }
    function _(D) {
      var P = D.lastIndexOf("google/protobuf/");
      if (P > -1) {
        var S = D.substring(P);
        if (S in r) return S;
      }
      return null;
    }
    function b(D, P) {
      try {
        if (d.isString(P) && P.charAt(0) === "{" && (P = JSON.parse(P)), !d.isString(P))
          E.setOptions(P.options).addJSON(P.nested);
        else {
          e.filename = D;
          var S = e(P, E, p), J, U = 0;
          if (S.imports)
            for (; U < S.imports.length; ++U)
              (J = _(S.imports[U]) || E.resolvePath(D, S.imports[U])) && I(J);
          if (S.weakImports)
            for (U = 0; U < S.weakImports.length; ++U)
              (J = _(S.weakImports[U]) || E.resolvePath(D, S.weakImports[U])) && I(J, !0);
        }
      } catch (T) {
        m(T);
      }
      !v && !C && m(null, E);
    }
    function I(D, P) {
      if (D = _(D) || D, !(E.files.indexOf(D) > -1)) {
        if (E.files.push(D), D in r) {
          v ? b(D, r[D]) : (++C, setTimeout(function() {
            --C, b(D, r[D]);
          }));
          return;
        }
        if (v) {
          var S;
          try {
            S = d.fs.readFileSync(D).toString("utf8");
          } catch (J) {
            P || m(J);
            return;
          }
          b(D, S);
        } else
          ++C, E.fetch(D, function(J, U) {
            if (--C, !!y) {
              if (J) {
                P ? C || m(null, E) : m(J);
                return;
              }
              b(D, U);
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
    return u.prototype.resolveAll.call(this);
  };
  var l = /^[A-Z]/;
  function s(a, o) {
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
    else if (o instanceof u) {
      for (var y = 0; y < /* initializes */
      o.nestedArray.length; ++y)
        this._handleRemove(o._nestedArray[y]);
      l.test(o.name) && delete o.parent[o.name];
    }
    delete this._fullyQualifiedObjects[o.fullName];
  }, i._configure = function(a, o, p) {
    n = a, e = o, r = p;
  }, root$1;
}
var hasRequiredUtil;
function requireUtil() {
  if (hasRequiredUtil) return util.exports;
  hasRequiredUtil = 1;
  var u = util.exports = requireMinimal(), f = requireRoots(), h, c;
  u.codegen = requireCodegen(), u.fetch = requireFetch(), u.path = requirePath(), u.fs = u.inquire("fs"), u.toArray = function(t) {
    if (t) {
      for (var l = Object.keys(t), s = new Array(l.length), a = 0; a < l.length; )
        s[a] = t[l[a++]];
      return s;
    }
    return [];
  }, u.toObject = function(t) {
    for (var l = {}, s = 0; s < t.length; ) {
      var a = t[s++], o = t[s++];
      o !== void 0 && (l[a] = o);
    }
    return l;
  };
  var d = /\\\\/g, n = /"/g;
  u.isReserved = function(t) {
    return /^(?:do|if|in|for|let|new|try|var|case|else|enum|eval|false|null|this|true|void|with|break|catch|class|const|super|throw|while|yield|delete|export|import|public|return|static|switch|typeof|default|extends|finally|package|private|continue|debugger|function|arguments|interface|protected|implements|instanceof)$/.test(t);
  }, u.safeProp = function(t) {
    return !/^[$\\w_]+$/.test(t) || u.isReserved(t) ? '["' + t.replace(d, "\\\\\\\\").replace(n, '\\\\"') + '"]' : "." + t;
  }, u.ucFirst = function(t) {
    return t.charAt(0).toUpperCase() + t.substring(1);
  };
  var e = /_([a-z])/g;
  u.camelCase = function(t) {
    return t.substring(0, 1) + t.substring(1).replace(e, function(l, s) {
      return s.toUpperCase();
    });
  }, u.compareFieldsById = function(t, l) {
    return t.id - l.id;
  }, u.decorateType = function(t, l) {
    if (t.$type)
      return l && t.$type.name !== l && (u.decorateRoot.remove(t.$type), t.$type.name = l, u.decorateRoot.add(t.$type)), t.$type;
    h || (h = requireType());
    var s = new h(l || t.name);
    return u.decorateRoot.add(s), s.ctor = t, Object.defineProperty(t, "$type", { value: s, enumerable: !1 }), Object.defineProperty(t.prototype, "$type", { value: s, enumerable: !1 }), s;
  };
  var r = 0;
  return u.decorateEnum = function(t) {
    if (t.$type)
      return t.$type;
    c || (c = require_enum());
    var l = new c("Enum" + r++, t);
    return u.decorateRoot.add(l), Object.defineProperty(t, "$type", { value: l, enumerable: !1 }), l;
  }, u.setProperty = function(t, l, s, a) {
    function o(p, y, E) {
      var v = y.shift();
      if (v === "__proto__" || v === "prototype")
        return p;
      if (y.length > 0)
        p[v] = o(p[v] || {}, y, E);
      else {
        var m = p[v];
        if (m && a)
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
  }, Object.defineProperty(u, "decorateRoot", {
    get: function() {
      return f.decorated || (f.decorated = new (requireRoot())());
    }
  }), util.exports;
}
var hasRequiredTypes;
function requireTypes() {
  return hasRequiredTypes || (hasRequiredTypes = 1, function(u) {
    var f = u, h = requireUtil(), c = [
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
  var u = requireObject();
  ((e.prototype = Object.create(u.prototype)).constructor = e).className = "Field";
  var f = require_enum(), h = requireTypes(), c = requireUtil(), d, n = /^required|optional|repeated$/;
  e.fromJSON = function(i, t) {
    var l = new e(i, t.id, t.type, t.rule, t.extend, t.options, t.comment);
    return t.edition && (l._edition = t.edition), l._defaultEdition = "proto3", l;
  };
  function e(r, i, t, l, s, a, o) {
    if (c.isObject(l) ? (o = s, a = l, l = s = void 0) : c.isObject(s) && (o = a, a = s, s = void 0), u.call(this, r, a), !c.isInteger(i) || i < 0)
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
    return u.prototype.setOption.call(this, i, t, l);
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
    return this.map ? this.defaultValue = c.emptyObject : this.repeated ? this.defaultValue = c.emptyArray : this.defaultValue = this.typeDefault, this.parent instanceof d && (this.parent.ctor.prototype[this.name] = this.defaultValue), u.prototype.resolve.call(this);
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
    return u.prototype._resolveFeatures.call(this, this._edition || i);
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
  var u = requireObject();
  ((c.prototype = Object.create(u.prototype)).constructor = c).className = "OneOf";
  var f = requireField(), h = requireUtil();
  function c(n, e, r, i) {
    if (Array.isArray(e) || (r = e, e = void 0), u.call(this, n, r), !(e === void 0 || Array.isArray(e)))
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
    u.prototype.onAdd.call(this, e);
    for (var r = this, i = 0; i < this.oneof.length; ++i) {
      var t = e.get(this.oneof[i]);
      t && !t.partOf && (t.partOf = r, r.fieldsArray.push(t));
    }
    d(this);
  }, c.prototype.onRemove = function(e) {
    for (var r = 0, i; r < this.fieldsArray.length; ++r)
      (i = this.fieldsArray[r]).parent && i.parent.remove(i);
    u.prototype.onRemove.call(this, e);
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
  const u = requireOneof();
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
      if (this.partOf instanceof u) {
        var a = Object.assign({}, this.partOf._features);
        this._features = Object.assign(a, s || {});
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
    var a = this.parsedOptions;
    if (s) {
      var o = a.find(function(E) {
        return Object.prototype.hasOwnProperty.call(E, t);
      });
      if (o) {
        var p = o[t];
        f.setProperty(p, s, l);
      } else
        o = {}, o[t] = f.setProperty({}, s, l), a.push(o);
    } else {
      var y = {};
      y[t] = l, a.push(y);
    }
    return this;
  }, r.prototype.setOptions = function(t, l) {
    if (t)
      for (var s = Object.keys(t), a = 0; a < s.length; ++a)
        this.setOption(s[a], t[s[a]], l);
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
  var u = requireObject();
  ((c.prototype = Object.create(u.prototype)).constructor = c).className = "Enum";
  var f = requireNamespace(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (u.call(this, d, e), n && typeof n != "object")
      throw TypeError("values must be an object");
    if (this.valuesById = {}, this.values = Object.create(this.valuesById), this.comment = r, this.comments = i || {}, this.valuesOptions = t, this._valuesFeatures = {}, this.reserved = void 0, n)
      for (var l = Object.keys(n), s = 0; s < l.length; ++s)
        typeof n[l[s]] == "number" && (this.valuesById[this.values[l[s]] = n[l[s]]] = l[s]);
  }
  return c.prototype._resolveFeatures = function(n) {
    return n = this._edition || n, u.prototype._resolveFeatures.call(this, n), Object.keys(this.values).forEach((e) => {
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
  var u = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n, e, r, i) {
    return e.delimited ? n("types[%i].encode(%s,w.uint32(%i)).uint32(%i)", r, i, (e.id << 3 | 3) >>> 0, (e.id << 3 | 4) >>> 0) : n("types[%i].encode(%s,w.uint32(%i).fork()).ldelim()", r, i, (e.id << 3 | 2) >>> 0);
  }
  function d(n) {
    for (var e = h.codegen(["m", "w"], n.name + "$encode")("if(!w)")("w=Writer.create()"), r, i, t = (
      /* initializes */
      n.fieldsArray.slice().sort(h.compareFieldsById)
    ), r = 0; r < t.length; ++r) {
      var l = t[r].resolve(), s = n._fieldsArray.indexOf(l), a = l.resolvedType instanceof u ? "int32" : l.type, o = f.basic[a];
      i = "m" + h.safeProp(l.name), l.map ? (e("if(%s!=null&&Object.hasOwnProperty.call(m,%j)){", i, l.name)("for(var ks=Object.keys(%s),i=0;i<ks.length;++i){", i)("w.uint32(%i).fork().uint32(%i).%s(ks[i])", (l.id << 3 | 2) >>> 0, 8 | f.mapKey[l.keyType], l.keyType), o === void 0 ? e("types[%i].encode(%s[ks[i]],w.uint32(18).fork()).ldelim().ldelim()", s, i) : e(".uint32(%i).%s(%s[ks[i]]).ldelim()", 16 | o, a, i), e("}")("}")) : l.repeated ? (e("if(%s!=null&&%s.length){", i, i), l.packed && f.packed[a] !== void 0 ? e("w.uint32(%i).fork()", (l.id << 3 | 2) >>> 0)("for(var i=0;i<%s.length;++i)", i)("w.%s(%s[i])", a, i)("w.ldelim()") : (e("for(var i=0;i<%s.length;++i)", i), o === void 0 ? c(e, l, s, i + "[i]") : e("w.uint32(%i).%s(%s[i])", (l.id << 3 | o) >>> 0, a, i)), e("}")) : (l.optional && e("if(%s!=null&&Object.hasOwnProperty.call(m,%j))", i, l.name), o === void 0 ? c(e, l, s, i) : e("w.uint32(%i).%s(%s)", (l.id << 3 | o) >>> 0, a, i));
    }
    return e("return w");
  }
  return encoder_1;
}
var hasRequiredIndexLight;
function requireIndexLight() {
  if (hasRequiredIndexLight) return indexLight.exports;
  hasRequiredIndexLight = 1;
  var u = indexLight.exports = requireIndexMinimal();
  u.build = "light";
  function f(c, d, n) {
    return typeof d == "function" ? (n = d, d = new u.Root()) : d || (d = new u.Root()), d.load(c, n);
  }
  u.load = f;
  function h(c, d) {
    return d || (d = new u.Root()), d.loadSync(c);
  }
  return u.loadSync = h, u.encoder = requireEncoder(), u.decoder = requireDecoder(), u.verifier = requireVerifier(), u.converter = requireConverter(), u.ReflectionObject = requireObject(), u.Namespace = requireNamespace(), u.Root = requireRoot(), u.Enum = require_enum(), u.Type = requireType(), u.Field = requireField(), u.OneOf = requireOneof(), u.MapField = requireMapfield(), u.Service = requireService(), u.Method = requireMethod(), u.Message = requireMessage(), u.wrappers = requireWrappers(), u.types = requireTypes(), u.util = requireUtil(), u.ReflectionObject._configure(u.Root), u.Namespace._configure(u.Type, u.Service, u.Enum), u.Root._configure(u.Type), u.Field._configure(u.Type), indexLight.exports;
}
var tokenize_1, hasRequiredTokenize;
function requireTokenize() {
  if (hasRequiredTokenize) return tokenize_1;
  hasRequiredTokenize = 1, tokenize_1 = l;
  var u = /[\\s{}=;:[\\],'"()<>]/g, f = /(?:"([^"\\\\]*(?:\\\\.[^"\\\\]*)*)")/g, h = /(?:'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)')/g, c = /^ *[*/]+ */, d = /^\\s*\\*?\\/*/, n = /\\n/g, e = /\\s/, r = /\\\\(.?)/g, i = {
    0: "\\0",
    r: "\\r",
    n: \`
\`,
    t: "	"
  };
  function t(s) {
    return s.replace(r, function(a, o) {
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
  function l(s, a) {
    s = s.toString();
    var o = 0, p = s.length, y = 1, E = 0, v = {}, m = [], _ = null;
    function b(k) {
      return Error("illegal " + k + " (line " + y + ")");
    }
    function I() {
      var k = _ === "'" ? h : f;
      k.lastIndex = o - 1;
      var L = k.exec(s);
      if (!L)
        throw b("string");
      return o = k.lastIndex, S(_), _ = null, t(L[1]);
    }
    function C(k) {
      return s.charAt(k);
    }
    function j(k, L, F) {
      var W = {
        type: s.charAt(k++),
        lineEmpty: !1,
        leading: F
      }, H;
      a ? H = 2 : H = 3;
      var B = k - H, $;
      do
        if (--B < 0 || ($ = s.charAt(B)) === \`
\`) {
          W.lineEmpty = !0;
          break;
        }
      while ($ === " " || $ === "	");
      for (var X = s.substring(k, L).split(n), z = 0; z < X.length; ++z)
        X[z] = X[z].replace(a ? d : c, "").trim();
      W.text = X.join(\`
\`).trim(), v[y] = W, E = y;
    }
    function K(k) {
      var L = D(k), F = s.substring(k, L), W = /^\\s*\\/\\//.test(F);
      return W;
    }
    function D(k) {
      for (var L = k; L < p && C(L) !== \`
\`; )
        L++;
      return L;
    }
    function P() {
      if (m.length > 0)
        return m.shift();
      if (_)
        return I();
      var k, L, F, W, H, B = o === 0;
      do {
        if (o === p)
          return null;
        for (k = !1; e.test(F = C(o)); )
          if (F === \`
\` && (B = !0, ++y), ++o === p)
            return null;
        if (C(o) === "/") {
          if (++o === p)
            throw b("comment");
          if (C(o) === "/")
            if (a) {
              if (W = o, H = !1, K(o - 1)) {
                H = !0;
                do
                  if (o = D(o), o === p || (o++, !B))
                    break;
                while (K(o));
              } else
                o = Math.min(p, D(o) + 1);
              H && (j(W, o, B), B = !0), y++, k = !0;
            } else {
              for (H = C(W = o + 1) === "/"; C(++o) !== \`
\`; )
                if (o === p)
                  return null;
              ++o, H && (j(W, o - 1, B), B = !0), ++y, k = !0;
            }
          else if ((F = C(o)) === "*") {
            W = o + 1, H = a || C(W) === "*";
            do {
              if (F === \`
\` && ++y, ++o === p)
                throw b("comment");
              L = F, F = C(o);
            } while (L !== "*" || F !== "/");
            ++o, H && (j(W, o - 2, B), B = !0), k = !0;
          } else
            return "/";
        }
      } while (k);
      var $ = o;
      u.lastIndex = 0;
      var X = u.test(C($++));
      if (!X)
        for (; $ < p && !u.test(C($)); )
          ++$;
      var z = s.substring(o, o = $);
      return (z === '"' || z === "'") && (_ = z), z;
    }
    function S(k) {
      m.push(k);
    }
    function J() {
      if (!m.length) {
        var k = P();
        if (k === null)
          return null;
        S(k);
      }
      return m[0];
    }
    function U(k, L) {
      var F = J(), W = F === k;
      if (W)
        return P(), !0;
      if (!L)
        throw b("token '" + F + "', '" + k + "' expected");
      return !1;
    }
    function T(k) {
      var L = null, F;
      return k === void 0 ? (F = v[y - 1], delete v[y - 1], F && (a || F.type === "*" || F.lineEmpty) && (L = F.leading ? F.text : null)) : (E < k && J(), F = v[k], delete v[k], F && !F.lineEmpty && (a || F.type === "/") && (L = F.leading ? null : F.text)), L;
    }
    return Object.defineProperty({
      next: P,
      peek: J,
      push: S,
      skip: U,
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
  var u = requireTokenize(), f = requireRoot(), h = requireType(), c = requireField(), d = requireMapfield(), n = requireOneof(), e = require_enum(), r = requireService(), i = requireMethod(), t = requireObject(), l = requireTypes(), s = requireUtil(), a = /^[1-9][0-9]*$/, o = /^-?[1-9][0-9]*$/, p = /^0[x][0-9a-fA-F]+$/, y = /^-?0[x][0-9a-fA-F]+$/, E = /^0[0-7]+$/, v = /^-?0[0-7]+$/, m = /^(?![eE])[0-9]*(?:\\.[0-9]*)?(?:[eE][+-]?[0-9]+)?$/, _ = /^[a-zA-Z_][a-zA-Z_0-9]*$/, b = /^(?:\\.?[a-zA-Z_][a-zA-Z_0-9]*)(?:\\.[a-zA-Z_][a-zA-Z_0-9]*)*$/;
  function I(C, j, K) {
    j instanceof f || (K = j, j = new f()), K || (K = I.defaults);
    var D = K.preferTrailingComment || !1, P = u(C, K.alternateCommentMode || !1), S = P.next, J = P.push, U = P.peek, T = P.skip, k = P.cmnt, L = !0, F, W, H, B = "proto2", $ = j, X = [], z = {}, ae = K.keepCase ? function(O) {
      return O;
    } : s.camelCase;
    function he() {
      X.forEach((O) => {
        O._edition = B, Object.keys(z).forEach((g) => {
          O.getOption(g) === void 0 && O.setOption(g, z[g], !0);
        });
      });
    }
    function N(O, g, R) {
      var A = I.filename;
      return R || (I.filename = null), Error("illegal " + (g || "token") + " '" + O + "' (" + (A ? A + ", " : "") + "line " + P.line + ")");
    }
    function ee() {
      var O = [], g;
      do {
        if ((g = S()) !== '"' && g !== "'")
          throw N(g);
        O.push(S()), T(g), g = U();
      } while (g === '"' || g === "'");
      return O.join("");
    }
    function ue(O) {
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
        if (b.test(g))
          return g;
        throw N(g, "value");
      }
    }
    function re(O, g) {
      var R, A;
      do
        if (g && ((R = U()) === '"' || R === "'")) {
          var w = ee();
          if (O.push(w), B >= 2023)
            throw N(w, "id");
        } else
          try {
            O.push([A = te(S()), T("to", !0) ? te(S()) : A]);
          } catch (x) {
            if (g && b.test(R) && B >= 2023)
              O.push(R);
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
    function pe(O, g) {
      var R = 1;
      switch (O.charAt(0) === "-" && (R = -1, O = O.substring(1)), O) {
        case "inf":
        case "INF":
        case "Inf":
          return R * (1 / 0);
        case "nan":
        case "NAN":
        case "Nan":
        case "NaN":
          return NaN;
        case "0":
          return 0;
      }
      if (a.test(O))
        return R * parseInt(O, 10);
      if (p.test(O))
        return R * parseInt(O, 16);
      if (E.test(O))
        return R * parseInt(O, 8);
      if (m.test(O))
        return R * parseFloat(O);
      throw N(O, "number", g);
    }
    function te(O, g) {
      switch (O) {
        case "max":
        case "MAX":
        case "Max":
          return 536870911;
        case "0":
          return 0;
      }
      if (!g && O.charAt(0) === "-")
        throw N(O, "id");
      if (o.test(O))
        return parseInt(O, 10);
      if (y.test(O))
        return parseInt(O, 16);
      if (v.test(O))
        return parseInt(O, 8);
      throw N(O, "id");
    }
    function ye() {
      if (F !== void 0)
        throw N("package");
      if (F = S(), !b.test(F))
        throw N(F, "name");
      $ = $.define(F), T(";");
    }
    function me() {
      var O = U(), g;
      switch (O) {
        case "option":
          if (B < "2024")
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
      O = ee(), T(";"), g.push(O);
    }
    function ve() {
      if (T("="), B = ee(), B < 2023)
        throw N(B, "syntax");
      T(";");
    }
    function ge() {
      if (T("="), B = ee(), !["2023", "2024"].includes(B))
        throw N(B, "edition");
      T(";");
    }
    function ie(O, g) {
      switch (g) {
        case "option":
          return Q(O, g), T(";"), !0;
        case "message":
          return ne(O, g), !0;
        case "enum":
          return de(O, g), !0;
        case "export":
        case "local":
          return B < "2024" || (g = S(), g === "export" || g === "local") || g !== "message" && g !== "enum" ? !1 : ie(O, g);
        case "service":
          return Ae(O, g), !0;
        case "extend":
          return Se(O, g), !0;
      }
      return !1;
    }
    function Z(O, g, R) {
      var A = P.line;
      if (O && (typeof O.comment != "string" && (O.comment = k()), O.filename = I.filename), T("{", !0)) {
        for (var w; (w = S()) !== "}"; )
          g(w);
        T(";", !0);
      } else
        R && R(), T(";"), O && (typeof O.comment != "string" || D) && (O.comment = k(A) || O.comment);
    }
    function ne(O, g) {
      if (!_.test(g = S()))
        throw N(g, "type name");
      var R = new h(g);
      Z(R, function(w) {
        if (!ie(R, w))
          switch (w) {
            case "map":
              Ee(R);
              break;
            case "required":
              if (B !== "proto2")
                throw N(w);
            /* eslint-disable no-fallthrough */
            case "repeated":
              Y(R, w);
              break;
            case "optional":
              if (B === "proto3")
                Y(R, "proto3_optional");
              else {
                if (B !== "proto2")
                  throw N(w);
                Y(R, "optional");
              }
              break;
            case "oneof":
              Oe(R, w);
              break;
            case "extensions":
              re(R.extensions || (R.extensions = []));
              break;
            case "reserved":
              re(R.reserved || (R.reserved = []), !0);
              break;
            default:
              if (B === "proto2" || !b.test(w))
                throw N(w);
              J(w), Y(R, "optional");
              break;
          }
      }), O.add(R), O === $ && X.push(R);
    }
    function Y(O, g, R) {
      var A = S();
      if (A === "group") {
        _e(O, g);
        return;
      }
      for (; A.endsWith(".") || U().startsWith("."); )
        A += S();
      if (!b.test(A))
        throw N(A, "type");
      var w = S();
      if (!_.test(w))
        throw N(w, "name");
      w = ae(w), T("=");
      var q = new c(w, te(S()), A, g, R);
      if (Z(q, function(M) {
        if (M === "option")
          Q(q, M), T(";");
        else
          throw N(M);
      }, function() {
        se(q);
      }), g === "proto3_optional") {
        var x = new n("_" + w);
        q.setOption("proto3_optional", !0), x.add(q), O.add(x);
      } else
        O.add(q);
      O === $ && X.push(q);
    }
    function _e(O, g) {
      if (B >= 2023)
        throw N("group");
      var R = S();
      if (!_.test(R))
        throw N(R, "name");
      var A = s.lcFirst(R);
      R === A && (R = s.ucFirst(R)), T("=");
      var w = te(S()), q = new h(R);
      q.group = !0;
      var x = new c(A, w, R, g);
      x.filename = I.filename, Z(q, function(M) {
        switch (M) {
          case "option":
            Q(q, M), T(";");
            break;
          case "required":
          case "repeated":
            Y(q, M);
            break;
          case "optional":
            B === "proto3" ? Y(q, "proto3_optional") : Y(q, "optional");
            break;
          case "message":
            ne(q, M);
            break;
          case "enum":
            de(q, M);
            break;
          case "reserved":
            re(q.reserved || (q.reserved = []), !0);
            break;
          case "export":
          case "local":
            if (B < "2024")
              throw N(M);
            switch (M = S(), M) {
              case "message":
                ne(q, M);
                break;
              case "enum":
                ne(q, M);
                break;
              default:
                throw N(M);
            }
            break;
          /* istanbul ignore next */
          default:
            throw N(M);
        }
      }), O.add(q).add(x);
    }
    function Ee(O) {
      T("<");
      var g = S();
      if (l.mapKey[g] === void 0)
        throw N(g, "type");
      T(",");
      var R = S();
      if (!b.test(R))
        throw N(R, "type");
      T(">");
      var A = S();
      if (!_.test(A))
        throw N(A, "name");
      T("=");
      var w = new d(ae(A), te(S()), g, R);
      Z(w, function(x) {
        if (x === "option")
          Q(w, x), T(";");
        else
          throw N(x);
      }, function() {
        se(w);
      }), O.add(w);
    }
    function Oe(O, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var R = new n(ae(g));
      Z(R, function(w) {
        w === "option" ? (Q(R, w), T(";")) : (J(w), Y(R, "optional"));
      }), O.add(R);
    }
    function de(O, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var R = new e(g);
      Z(R, function(w) {
        switch (w) {
          case "option":
            Q(R, w), T(";");
            break;
          case "reserved":
            re(R.reserved || (R.reserved = []), !0), R.reserved === void 0 && (R.reserved = []);
            break;
          default:
            Re(R, w);
        }
      }), O.add(R), O === $ && X.push(R);
    }
    function Re(O, g) {
      if (!_.test(g))
        throw N(g, "name");
      T("=");
      var R = te(S(), !0), A = {
        options: void 0
      };
      A.getOption = function(w) {
        return this.options[w];
      }, A.setOption = function(w, q) {
        t.prototype.setOption.call(A, w, q);
      }, A.setParsedOption = function() {
      }, Z(A, function(q) {
        if (q === "option")
          Q(A, q), T(";");
        else
          throw N(q);
      }, function() {
        se(A);
      }), O.add(g, R, A.comment, A.parsedOptions || A.options);
    }
    function Q(O, g) {
      var R, A, w = !0;
      for (g === "option" && (g = S()); g !== "="; ) {
        if (g === "(") {
          var q = S();
          T(")"), g = "(" + q + ")";
        }
        if (w) {
          if (w = !1, g.includes(".") && !g.includes("(")) {
            var x = g.split(".");
            R = x[0] + ".", g = x[1];
            continue;
          }
          R = g;
        } else
          A = A ? A += g : g;
        g = S();
      }
      var G = A ? R.concat(A) : R, M = ce(O, G);
      A = A && A[0] === "." ? A.slice(1) : A, R = R && R[R.length - 1] === "." ? R.slice(0, -1) : R, be(O, R, M, A);
    }
    function ce(O, g) {
      if (T("{", !0)) {
        for (var R = {}; !T("}", !0); ) {
          if (!_.test(V = S()))
            throw N(V, "name");
          if (V === null)
            throw N(V, "end of input");
          var A, w = V;
          if (T(":", !0), U() === "{")
            A = ce(O, g + "." + V);
          else if (U() === "[") {
            A = [];
            var q;
            if (T("[", !0)) {
              do
                q = ue(), A.push(q);
              while (T(",", !0));
              T("]"), typeof q < "u" && fe(O, g + "." + V, q);
            }
          } else
            A = ue(), fe(O, g + "." + V, A);
          var x = R[w];
          x && (A = [].concat(x).concat(A)), R[w] = A, T(",", !0), T(";", !0);
        }
        return R;
      }
      var G = ue();
      return fe(O, g, G), G;
    }
    function fe(O, g, R) {
      if ($ === O && /^features\\./.test(g)) {
        z[g] = R;
        return;
      }
      O.setOption && O.setOption(g, R);
    }
    function be(O, g, R, A) {
      O.setParsedOption && O.setParsedOption(g, R, A);
    }
    function se(O) {
      if (T("[", !0)) {
        do
          Q(O, "option");
        while (T(",", !0));
        T("]");
      }
      return O;
    }
    function Ae(O, g) {
      if (!_.test(g = S()))
        throw N(g, "service name");
      var R = new r(g);
      Z(R, function(w) {
        if (!ie(R, w))
          if (w === "rpc")
            we(R, w);
          else
            throw N(w);
      }), O.add(R), O === $ && X.push(R);
    }
    function we(O, g) {
      var R = k(), A = g;
      if (!_.test(g = S()))
        throw N(g, "name");
      var w = g, q, x, G, M;
      if (T("("), T("stream", !0) && (x = !0), !b.test(g = S()) || (q = g, T(")"), T("returns"), T("("), T("stream", !0) && (M = !0), !b.test(g = S())))
        throw N(g);
      G = g, T(")");
      var oe = new i(w, A, q, G, x, M);
      oe.comment = R, Z(oe, function(le) {
        if (le === "option")
          Q(oe, le), T(";");
        else
          throw N(le);
      }), O.add(oe);
    }
    function Se(O, g) {
      if (!b.test(g = S()))
        throw N(g, "reference");
      var R = g;
      Z(null, function(w) {
        switch (w) {
          case "required":
          case "repeated":
            Y(O, w, R);
            break;
          case "optional":
            B === "proto3" ? Y(O, "proto3_optional", R) : Y(O, "optional", R);
            break;
          default:
            if (B === "proto2" || !b.test(w))
              throw N(w);
            J(w), Y(O, "optional", R);
            break;
        }
      });
    }
    for (var V; (V = S()) !== null; )
      switch (V) {
        case "package":
          if (!L)
            throw N(V);
          ye();
          break;
        case "import":
          if (!L)
            throw N(V);
          me();
          break;
        case "syntax":
          if (!L)
            throw N(V);
          ve();
          break;
        case "edition":
          if (!L)
            throw N(V);
          ge();
          break;
        case "option":
          Q($, V), T(";", !0);
          break;
        default:
          if (ie($, V)) {
            L = !1;
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
  var u = /\\/|\\./;
  function f(c, d) {
    u.test(c) || (c = "google/protobuf/" + c + ".proto", d = { nested: { google: { nested: { protobuf: { nested: d } } } } }), f[c] = d;
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
  var u = src.exports = requireIndexLight();
  return u.build = "full", u.tokenize = requireTokenize(), u.parse = requireParse(), u.common = requireCommon(), u.Root._configure(u.Type, u.parse, u.common), src.exports;
}
var protobufjs, hasRequiredProtobufjs;
function requireProtobufjs() {
  return hasRequiredProtobufjs || (hasRequiredProtobufjs = 1, protobufjs = requireSrc()), protobufjs;
}
var protobufjsExports = requireProtobufjs(), StateStreamErrorCode = /* @__PURE__ */ ((u) => (u.CONNECTION_FAILED = "CONNECTION_FAILED", u.RECONNECT_FAILED = "RECONNECT_FAILED", u.CONNECTION_LOST = "CONNECTION_LOST", u.CONNECTION_TIMEOUT = "CONNECTION_TIMEOUT", u.AUTH_FAILED = "AUTH_FAILED", u.AUTH_REFRESH_FAILED = "AUTH_REFRESH_FAILED", u.DEVICE_ERROR = "DEVICE_ERROR", u.DECODE_ERROR = "DECODE_ERROR", u.FRAME_PROCESS_ERROR = "FRAME_PROCESS_ERROR", u.STREAM_ALREADY_STARTED = "STREAM_ALREADY_STARTED", u.WORKER_INIT_FAILED = "WORKER_INIT_FAILED", u.UNKNOWN_ERROR = "UNKNOWN_ERROR", u))(StateStreamErrorCode || {}), ConnectionStatus = /* @__PURE__ */ ((u) => (u.DISCONNECTED = "DISCONNECTED", u.CONNECTING = "CONNECTING", u.CONNECTED = "CONNECTED", u.RECONNECTING = "RECONNECTING", u))(ConnectionStatus || {}), AuthStatus = /* @__PURE__ */ ((u) => (u.UNAUTHENTICATED = "UNAUTHENTICATED", u.AUTHENTICATING = "AUTHENTICATING", u.AUTHENTICATED = "AUTHENTICATED", u.FAILED = "FAILED", u))(AuthStatus || {});
const nested = { BSB_State: { nested: { StateUpdate: { oneofs: { state: { oneof: ["deviceName", "power", "brightness", "audioVolume", "wifi", "updateState", "updateCheck", "timezone", "matter", "frame", "input", "timer", "ble", "autoUpdateState"] } }, fields: { deviceName: { type: "BSB_State.DeviceName", id: 1 }, power: { type: "BSB_State.Power", id: 2 }, brightness: { type: "BSB_State.Brightness", id: 3 }, audioVolume: { type: "BSB_State.AudioVolume", id: 4 }, wifi: { type: "BSB_State.Wifi", id: 5 }, updateState: { type: "BSB_Update.UpdateState", id: 6 }, updateCheck: { type: "BSB_Update.CheckState", id: 7 }, timezone: { type: "BSB_State.Timezone", id: 8 }, matter: { type: "BSB_State.Matter", id: 9 }, frame: { type: "BSB_Frame.Frame", id: 10 }, input: { type: "BSB_Input.InputEvent", id: 11 }, timer: { type: "BSB_Timer.Timer", id: 12 }, ble: { type: "BSB_State.Ble.Ble", id: 13 }, autoUpdateState: { type: "BSB_Update.AutoUpdateState", id: 14 } } }, State: { oneofs: { _error: { oneof: ["error"] } }, fields: { timestamp: { type: "fixed64", id: 1 }, updates: { rule: "repeated", type: "StateUpdate", id: 2 }, error: { type: "BSB_Error.Error", id: 3, options: { proto3_optional: !0 } } } }, DeviceName: { fields: { name: { type: "string", id: 1 } } }, BrightnessAutomatic: { fields: {} }, BrightnessManual: { fields: { brightness: { type: "uint32", id: 1 } } }, Brightness: { oneofs: { setting: { oneof: ["automatic", "manual"] } }, fields: { automatic: { type: "BrightnessAutomatic", id: 1 }, manual: { type: "BrightnessManual", id: 2 }, actualBrightness: { type: "uint32", id: 3 } } }, BatteryStatus: { values: { DISCHARGING: 0, CHARGING: 1, CHARGED: 2 } }, UnknownPowerState: { fields: {} }, PowerState: { fields: { batteryStatus: { type: "BatteryStatus", id: 1 }, batteryChargePercent: { type: "uint32", id: 2 }, batteryVoltageMv: { type: "uint32", id: 3 }, batteryCurrentMa: { type: "sint32", id: 4 }, usbVoltageMv: { type: "uint32", id: 5 } } }, Power: { oneofs: { state: { oneof: ["unknown", "known"] } }, fields: { unknown: { type: "UnknownPowerState", id: 1 }, known: { type: "PowerState", id: 2 } } }, AudioVolume: { fields: { volume: { type: "uint32", id: 1 } } }, WifiConnectionStatus: { values: { CONNECTED: 0, CONNECTING: 1, DISCONNECTING: 2, RECONNECTING: 3 } }, WifiSecurity: { values: { UNKNOWN: 0, OPEN: 1, WPA: 2, WPA2: 3, WEP: 4, WPA_WPA2: 5, WPA3: 6, WPA2_WPA3: 7 } }, IpConfigurationMethod: { values: { DHCP: 0, STATIC: 1 } }, IpProtocol: { values: { IPV4: 0, IPV6: 1 } }, WifiStateUnknown: { fields: {} }, WifiStateDisconnected: { fields: {} }, WifiStateConnected: { fields: { status: { type: "WifiConnectionStatus", id: 1 }, ssid: { type: "string", id: 2 }, bssid: { type: "string", id: 3 }, channel: { type: "uint32", id: 4 }, rssi: { type: "sint32", id: 5 }, security: { type: "WifiSecurity", id: 6 } } }, IpAddress: { fields: { protocol: { type: "IpProtocol", id: 1 }, method: { type: "IpConfigurationMethod", id: 2 }, address: { type: "string", id: 3 }, gateway: { type: "string", id: 4 }, netmask: { type: "string", id: 5 } } }, Wifi: { oneofs: { wifiState: { oneof: ["unknown", "disconnected", "connected"] } }, fields: { unknown: { type: "WifiStateUnknown", id: 1 }, disconnected: { type: "WifiStateDisconnected", id: 2 }, connected: { type: "WifiStateConnected", id: 3 }, ipAddresses: { rule: "repeated", type: "IpAddress", id: 4 } } }, Timezone: { fields: { name: { type: "string", id: 1 }, offset: { type: "sint32", id: 2 }, abbr: { type: "string", id: 3 } } }, MatterCommissioningStatus: { values: { NEVER_STARTED: 0, STARTED: 1, COMPLETED_SUCCESSFULLY: 2, FAILED: 3 } }, MatterCommissioningState: { fields: { status: { type: "MatterCommissioningStatus", id: 1 }, timestamp: { type: "fixed64", id: 2 } } }, Matter: { fields: { fabricCount: { type: "uint32", id: 1 }, state: { type: "MatterCommissioningState", id: 2 } } }, Ble: { nested: { ServiceStatus: { values: { RESET: 0, INITIALIZATION: 1, READY: 2, ADVERTISING: 3, CONNECTABLE: 4, CONNECTED: 5, ERROR: 6 } }, Ble: { oneofs: { _remoteAddress: { oneof: ["remoteAddress"] } }, fields: { status: { type: "ServiceStatus", id: 1 }, remoteAddress: { type: "string", id: 2, options: { proto3_optional: !0 } } } } } } } }, BSB_Update: { nested: { UpdateEvent: { values: { SESSION_START: 0, SESSION_STOP: 1, ACTION_BEGIN: 2, ACTION_DONE: 3, DETAIL_CHANGE: 4, ACTION_PROGRESS: 5, EVENT_NONE: 6 } }, UpdateAction: { values: { DOWNLOAD: 0, SHA_VERIFICATION: 1, UNPACK: 2, INSTALLATION_PREPARE: 3, INSTALLATION_APPLY: 4, ACTION_NONE: 5 } }, UpdateStatus: { values: { OK: 0, BATTERY_LOW: 1, BUSY: 2, DOWNLOAD_FAILURE: 3, DOWNLOAD_ABORT: 4, SHA_MISMATCH: 5, UNPACK_CREATE_STAGING_DIRECTORY_FAILURE: 6, UNPACK_ARCHIVE_OPEN_FAILURE: 7, UNPACK_ARCHIVE_UNPACK_FAILURE: 8, INSTALLATION_PREPARE_MANIFEST_NOT_FOUND: 9, INSTALLATION_PREPARE_MANIFEST_INVALID: 10, INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE: 11, INSTALLATION_PREPARE_POINTER_SETUP_FAILURE: 12, UNKNOWN_FAILURE: 13 } }, CheckError: { values: { NOT_AVAILABLE: 0, FAILURE: 1, IDLE: 2 } }, UpdateAvailable: { fields: { version: { type: "string", id: 1 } } }, UpdateUnavailable: { fields: { reason: { type: "CheckError", id: 1 } } }, UpdateState: { fields: { event: { type: "UpdateEvent", id: 1 }, action: { type: "UpdateAction", id: 2 }, status: { type: "UpdateStatus", id: 3 } } }, CheckState: { oneofs: { status: { oneof: ["available", "unavailable"] } }, fields: { available: { type: "UpdateAvailable", id: 1 }, unavailable: { type: "UpdateUnavailable", id: 2 } } }, AutoUpdateInterval: { fields: { start: { type: "uint32", id: 1 }, end: { type: "uint32", id: 2 } } }, AutoUpdateState: { fields: { enabled: { type: "bool", id: 1 }, interval: { type: "AutoUpdateInterval", id: 2 } } } } }, BSB_Frame: { nested: { Encoding: { values: { PLAIN: 0, RUN_LENGTH: 1, DEFLATE: 2, DEFLATE_RUN_LENGTH: 3 } }, PixelFormat: { values: { RGB888: 0, L8: 1, L4: 2 } }, Screen: { values: { FRONT: 0, BACK: 1 } }, Frame: { fields: { screen: { type: "Screen", id: 1 }, width: { type: "uint32", id: 2 }, height: { type: "uint32", id: 3 }, encoding: { type: "Encoding", id: 4 }, pixelFormat: { type: "PixelFormat", id: 5 }, data: { type: "bytes", id: 6 } } } } }, BSB_Timer: { nested: { Timer: { fields: { json: { type: "BSB_Util.Json", id: 1 } } } } }, BSB_Util: { nested: { Compression: { values: { PLAIN: 0, GZIP: 1 } }, Json: { fields: { compression: { type: "Compression", id: 1 }, data: { type: "bytes", id: 2 } } } } }, BSB_Input: { nested: { Button: { values: { OK: 0, BACK: 1, START: 2 } }, ButtonAction: { values: { PRESS: 0, RELEASE: 1 } }, SwitchPosition: { values: { BUSY: 0, CUSTOM: 1, OFF: 2, APPS: 3, SETTINGS: 4 } }, ButtonEvent: { fields: { button: { type: "Button", id: 1 }, action: { type: "ButtonAction", id: 2 } } }, SwitchEvent: { fields: { position: { type: "SwitchPosition", id: 1 } } }, EncoderEvent: { fields: { delta: { type: "sint32", id: 1 } } }, InputEvent: { oneofs: { event: { oneof: ["buttonEvent", "switchEvent", "encoderEvent"] } }, fields: { buttonEvent: { type: "ButtonEvent", id: 1 }, switchEvent: { type: "SwitchEvent", id: 2 }, encoderEvent: { type: "EncoderEvent", id: 3 } } } } }, BSB_Error: { nested: { Cause: { values: { RESOURCE_LIMIT: 0 } }, Severity: { values: { FATAL: 0, ERROR: 1, WARNING: 2 } }, Error: { fields: { cause: { type: "Cause", id: 1 }, severity: { type: "Severity", id: 2 } } } } } };
var bundle = {
  nested
};
function decompressRLE(u, f) {
  const h = [];
  for (let c = 0; c < u.length; ) {
    const d = u[c++];
    if (d === void 0) break;
    const n = d & 127;
    if (!n)
      continue;
    if (d & 128) {
      const r = n * f, i = u.subarray(c, c + r);
      for (let t = 0; t < i.length; t++)
        h.push(i[t]);
      c += r;
      continue;
    }
    const e = u.subarray(c, c + f);
    c += f;
    for (let r = 0; r < n; r++)
      for (let i = 0; i < f; i++)
        h.push(e[i]);
  }
  return new Uint8Array(h);
}
async function decompressDeflate(u) {
  if (typeof DecompressionStream > "u")
    throw new Error("DecompressionStream is not supported in this environment.");
  try {
    const f = new DecompressionStream("deflate"), h = f.writable.getWriter();
    h.write(u), h.close();
    const d = await new Response(f.readable).arrayBuffer();
    return new Uint8Array(d);
  } catch (f) {
    throw new Error(\`Deflate decompression failed: \${f instanceof Error ? f.message : String(f)}\`);
  }
}
function convertL4toRGBA(u, f, h) {
  const c = new Uint8ClampedArray(f * h * 4);
  let d = 0;
  for (let n = 0; n < u.length; n++) {
    const e = u[n], r = (e & 15) * 17, i = (e >> 4 & 15) * 17, t = [r, i];
    for (const l of t)
      if (d < f * h) {
        const s = d * 4;
        c[s] = l, c[s + 1] = l, c[s + 2] = l, c[s + 3] = 255, d++;
      }
  }
  return c;
}
function convertL8toRGBA(u, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = Math.min(u.length, f * h);
  for (let n = 0; n < d; n++) {
    const e = u[n], r = n * 4;
    c[r] = e, c[r + 1] = e, c[r + 2] = e, c[r + 3] = 255;
  }
  return c;
}
function convertRGB888toRGBA(u, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = f * h;
  for (let n = 0; n < d; n++) {
    const e = n * 3, r = n * 4;
    e + 2 < u.length && (c[r] = u[e + 2], c[r + 1] = u[e + 1], c[r + 2] = u[e], c[r + 3] = 255);
  }
  return c;
}
async function processFrame(u) {
  if (!u.data || !u.width || !u.height)
    return null;
  let f = u.data;
  const h = u.pixelFormat === 0 ? 3 : 1;
  switch (u.encoding) {
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
  switch (u.pixelFormat) {
    case 2:
      return convertL4toRGBA(f, u.width, u.height);
    case 1:
      return convertL8toRGBA(f, u.width, u.height);
    case 0:
      return convertRGB888toRGBA(f, u.width, u.height);
    default:
      return new Uint8ClampedArray(u.width * u.height * 4);
  }
}
const root = protobufjsExports.Root.fromJSON(bundle), StateType = root.lookupType("BSB_State.State"), AUTH_CODE = 3e3, RECONNECT_CODES = /* @__PURE__ */ new Set([1001, 1006, 1012, 1013, 1014, 3008]), MAX_AUTH_ATTEMPTS = 5, MAX_RECONNECT_ATTEMPTS = 5;
let socket = null, isBinaryMode = !0, currentMode = "local", currentToken, currentAddr = "", retryCount = 0, authRetryCount = 0, isAuthReported = !1, stabilityTimeout;
const activePorts = /* @__PURE__ */ new Set(), subscriptions = /* @__PURE__ */ new Map();
let processingQueue = Promise.resolve();
function broadcast(u) {
  for (const f of activePorts)
    f.postMessage(u);
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
function stopAndCleanup() {
  socket && (socket.close(), socket = null), stabilityTimeout && (clearTimeout(stabilityTimeout), stabilityTimeout = void 0), subscriptions.clear(), activePorts.clear(), retryCount = 0, authRetryCount = 0, isAuthReported = !1;
}
function connect(u, f, h = !0, c = "local") {
  socket && socket.close(), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.CONNECTING }), currentAddr = u, isBinaryMode = h, currentMode = c, currentToken = f, isAuthReported = !1;
  const d = new URL(u);
  socket = new WebSocket(d.toString()), socket.binaryType = "arraybuffer", socket.onopen = () => {
    broadcast({ type: "CONNECTED" }), currentMode === "local" && (socket == null || socket.send(JSON.stringify({ enable: !0 }))), sendAuth(), stabilityTimeout && clearTimeout(stabilityTimeout), stabilityTimeout = setTimeout(() => {
      retryCount = 0, authRetryCount = 0, console.log("[Worker] Connection stable. All retry counters reset.");
    }, 5e3), currentMode === "remote" && subscriptions.size > 0 && sendSubscriptions(), currentMode === "local" && broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED });
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
            const { cause: s, severity: a } = l.error;
            if (s != null && a != null) {
              const o = root.lookupEnum("BSB_Error.Cause"), p = root.lookupEnum("BSB_Error.Severity"), y = o.valuesById[s] || "UNKNOWN", E = p.valuesById[a] || "UNKNOWN";
              if (broadcast({
                type: "ERROR",
                code: StateStreamErrorCode.DEVICE_ERROR,
                message: \`Server reported \${E}: \${y}\`,
                data: l.error
              }), a === p.values.FATAL) {
                stopAndCleanup();
                return;
              }
              if (a === p.values.ERROR)
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
              const a = s.frame;
              if (a && a.data)
                try {
                  const o = await processFrame(a);
                  o && (a.data = o);
                } catch (o) {
                  broadcast({
                    type: "ERROR",
                    code: StateStreamErrorCode.FRAME_PROCESS_ERROR,
                    message: o instanceof Error ? o.message : String(o),
                    data: a.data
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
    if (console.log("[Worker] Socket closed:", n), stabilityTimeout && (clearTimeout(stabilityTimeout), stabilityTimeout = void 0), !socket || activePorts.size === 0) {
      console.log("[Worker] Connection closed or no active ports. No retries.");
      return;
    }
    if (n.code === AUTH_CODE && currentMode === "remote") {
      authRetryCount < MAX_AUTH_ATTEMPTS ? (authRetryCount++, console.warn(\`[Worker] Auth failed (3000). Requesting new token... (Attempt \${authRetryCount}/\${MAX_AUTH_ATTEMPTS})\`), broadcast({ type: "TOKEN_EXPIRED" })) : (broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.FAILED }), broadcast({
        type: "ERROR",
        code: StateStreamErrorCode.AUTH_FAILED,
        message: \`Maximum authentication attempts (\${MAX_AUTH_ATTEMPTS}) reached. Please log in again.\`
      }));
      return;
    }
    if (currentMode === "remote" && RECONNECT_CODES.has(n.code)) {
      if (retryCount < MAX_RECONNECT_ATTEMPTS) {
        retryCount++;
        const e = Math.min(1e3 * retryCount, 5e3);
        console.log(\`[Worker] Reconnecting (network code: \${n.code}) in \${e}ms... (Attempt \${retryCount}/\${MAX_RECONNECT_ATTEMPTS})\`), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.RECONNECTING }), setTimeout(() => {
          activePorts.size > 0 && socket && connect(currentAddr, currentToken, isBinaryMode, currentMode);
        }, e);
      } else
        broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), broadcast({
          type: "ERROR",
          code: StateStreamErrorCode.RECONNECT_FAILED,
          message: \`Maximum reconnection attempts (\${MAX_RECONNECT_ATTEMPTS}) reached. Connection lost.\`
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
function handleCommand(u, f) {
  switch (u.type) {
    case "START":
      activePorts.add(f), socket && socket.readyState === WebSocket.OPEN && currentAddr === u.addr ? (f.postMessage({ type: "CONNECTED" }), f.postMessage({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED })) : connect(u.addr, u.token, u.isBinary, u.mode);
      break;
    case "STOP":
      activePorts.delete(f);
      for (const [e, r] of subscriptions.entries())
        r.delete(f), r.size === 0 && (subscriptions.delete(e), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [e] })));
      f.postMessage({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), activePorts.size === 0 && stopAndCleanup();
      break;
    case "UPDATE_TOKEN":
      const h = currentToken;
      if (currentToken = u.token, currentMode === "remote") {
        const e = socket && socket.readyState === WebSocket.OPEN;
        if (e && h === u.token)
          return;
        e ? sendAuth() : h !== u.token && currentAddr && activePorts.size > 0 && connect(currentAddr, currentToken, isBinaryMode, currentMode);
      }
      break;
    case "SUBSCRIBE":
      let c = subscriptions.get(u.guid);
      c || (c = /* @__PURE__ */ new Set(), subscriptions.set(u.guid, c));
      const d = c.size === 0;
      c.add(f), d && (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ subscribe: [u.guid] }));
      break;
    case "UNSUBSCRIBE":
      const n = subscriptions.get(u.guid);
      n && (n.delete(f), n.size === 0 && (subscriptions.delete(u.guid), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [u.guid] }))));
      break;
  }
}
if ("SharedWorkerGlobalScope" in self) {
  const u = self;
  u.onconnect = (f) => {
    const h = f.ports[0];
    h && (h.onmessage = (c) => handleCommand(c.data, h), h.start());
  };
} else {
  const u = self;
  u.onmessage = (f) => {
    handleCommand(f.data, u);
  };
}
`,Se=typeof self<"u"&&self.Blob&&new Blob(["URL.revokeObjectURL(import.meta.url);",ze],{type:"text/javascript;charset=utf-8"});function Br(t){let e;try{if(e=Se&&(self.URL||self.webkitURL).createObjectURL(Se),!e)throw"";const r=new Worker(e,{type:"module",name:t?.name});return r.addEventListener("error",()=>{(self.URL||self.webkitURL).revokeObjectURL(e)}),r}catch{return new Worker("data:text/javascript;charset=utf-8,"+encodeURIComponent(ze),{type:"module",name:t?.name})}}const Fr=`var commonjsGlobal = typeof globalThis < "u" ? globalThis : typeof window < "u" ? window : typeof global < "u" ? global : typeof self < "u" ? self : {}, src = { exports: {} }, indexLight = { exports: {} }, indexMinimal = {}, minimal = {}, aspromise, hasRequiredAspromise;
function requireAspromise() {
  if (hasRequiredAspromise) return aspromise;
  hasRequiredAspromise = 1, aspromise = u;
  function u(f, h) {
    for (var c = new Array(arguments.length - 1), d = 0, n = 2, e = !0; n < arguments.length; )
      c[d++] = arguments[n++];
    return new Promise(function(i, t) {
      c[d] = function(s) {
        if (e)
          if (e = !1, s)
            t(s);
          else {
            for (var a = new Array(arguments.length - 1), o = 0; o < a.length; )
              a[o++] = arguments[o];
            i.apply(null, a);
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
  return hasRequiredBase64 || (hasRequiredBase64 = 1, function(u) {
    var f = u;
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
      for (var l = null, s = [], a = 0, o = 0, p; i < t; ) {
        var y = r[i++];
        switch (o) {
          case 0:
            s[a++] = h[y >> 2], p = (y & 3) << 4, o = 1;
            break;
          case 1:
            s[a++] = h[p | y >> 4], p = (y & 15) << 2, o = 2;
            break;
          case 2:
            s[a++] = h[p | y >> 6], s[a++] = h[y & 63], o = 0;
            break;
        }
        a > 8191 && ((l || (l = [])).push(String.fromCharCode.apply(String, s)), a = 0);
      }
      return o && (s[a++] = h[p], s[a++] = 61, o === 1 && (s[a++] = 61)), l ? (a && l.push(String.fromCharCode.apply(String, s.slice(0, a))), l.join("")) : String.fromCharCode.apply(String, s.slice(0, a));
    };
    var n = "invalid encoding";
    f.decode = function(r, i, t) {
      for (var l = t, s = 0, a, o = 0; o < r.length; ) {
        var p = r.charCodeAt(o++);
        if (p === 61 && s > 1)
          break;
        if ((p = c[p]) === void 0)
          throw Error(n);
        switch (s) {
          case 0:
            a = p, s = 1;
            break;
          case 1:
            i[t++] = a << 2 | (p & 48) >> 4, a = p, s = 2;
            break;
          case 2:
            i[t++] = (a & 15) << 4 | (p & 60) >> 2, a = p, s = 3;
            break;
          case 3:
            i[t++] = (a & 3) << 6 | p, s = 0;
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
  hasRequiredEventemitter = 1, eventemitter = u;
  function u() {
    this._listeners = {};
  }
  return u.prototype.on = function(h, c, d) {
    return (this._listeners[h] || (this._listeners[h] = [])).push({
      fn: c,
      ctx: d || this
    }), this;
  }, u.prototype.off = function(h, c) {
    if (h === void 0)
      this._listeners = {};
    else if (c === void 0)
      this._listeners[h] = [];
    else
      for (var d = this._listeners[h], n = 0; n < d.length; )
        d[n].fn === c ? d.splice(n, 1) : ++n;
    return this;
  }, u.prototype.emit = function(h) {
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
  hasRequiredFloat = 1, float = u(u);
  function u(n) {
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
      function a(o, p) {
        return r[3] = o[p], r[2] = o[p + 1], r[1] = o[p + 2], r[0] = o[p + 3], e[0];
      }
      n.readFloatLE = i ? s : a, n.readFloatBE = i ? a : s;
    }() : function() {
      function e(i, t, l, s) {
        var a = t < 0 ? 1 : 0;
        if (a && (t = -t), t === 0)
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
          i((a << 31 | 2139095040) >>> 0, l, s);
        else if (t < 11754943508222875e-54)
          i((a << 31 | Math.round(t / 1401298464324817e-60)) >>> 0, l, s);
        else {
          var o = Math.floor(Math.log(t) / Math.LN2), p = Math.round(t * Math.pow(2, -o) * 8388608) & 8388607;
          i((a << 31 | o + 127 << 23 | p) >>> 0, l, s);
        }
      }
      n.writeFloatLE = e.bind(null, f), n.writeFloatBE = e.bind(null, h);
      function r(i, t, l) {
        var s = i(t, l), a = (s >> 31) * 2 + 1, o = s >>> 23 & 255, p = s & 8388607;
        return o === 255 ? p ? NaN : a * (1 / 0) : o === 0 ? a * 1401298464324817e-60 * p : a * Math.pow(2, o - 150) * (p + 8388608);
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
      function a(o, p) {
        return r[7] = o[p], r[6] = o[p + 1], r[5] = o[p + 2], r[4] = o[p + 3], r[3] = o[p + 4], r[2] = o[p + 5], r[1] = o[p + 6], r[0] = o[p + 7], e[0];
      }
      n.readDoubleLE = i ? s : a, n.readDoubleBE = i ? a : s;
    }() : function() {
      function e(i, t, l, s, a, o) {
        var p = s < 0 ? 1 : 0;
        if (p && (s = -s), s === 0)
          i(0, a, o + t), i(1 / s > 0 ? (
            /* positive */
            0
          ) : (
            /* negative 0 */
            2147483648
          ), a, o + l);
        else if (isNaN(s))
          i(0, a, o + t), i(2146959360, a, o + l);
        else if (s > 17976931348623157e292)
          i(0, a, o + t), i((p << 31 | 2146435072) >>> 0, a, o + l);
        else {
          var y;
          if (s < 22250738585072014e-324)
            y = s / 5e-324, i(y >>> 0, a, o + t), i((p << 31 | y / 4294967296) >>> 0, a, o + l);
          else {
            var E = Math.floor(Math.log(s) / Math.LN2);
            E === 1024 && (E = 1023), y = s * Math.pow(2, -E), i(y * 4503599627370496 >>> 0, a, o + t), i((p << 31 | E + 1023 << 20 | y * 1048576 & 1048575) >>> 0, a, o + l);
          }
        }
      }
      n.writeDoubleLE = e.bind(null, f, 0, 4), n.writeDoubleBE = e.bind(null, h, 4, 0);
      function r(i, t, l, s, a) {
        var o = i(s, a + t), p = i(s, a + l), y = (p >> 31) * 2 + 1, E = p >>> 20 & 2047, v = 4294967296 * (p & 1048575) + o;
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
    } catch (u) {
    }
    return null;
  }
  return inquire_1;
}
var utf8 = {}, hasRequiredUtf8;
function requireUtf8() {
  return hasRequiredUtf8 || (hasRequiredUtf8 = 1, function(u) {
    var f = u;
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
  hasRequiredPool = 1, pool_1 = u;
  function u(f, h, c) {
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
  var u = requireMinimal();
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
    if (u.isString(e))
      if (u.Long)
        e = u.Long.fromString(e);
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
    return u.Long ? new u.Long(this.lo | 0, this.hi | 0, !!e) : { low: this.lo | 0, high: this.hi | 0, unsigned: !!e };
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
  return hasRequiredMinimal || (hasRequiredMinimal = 1, function(u) {
    var f = u;
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
  var u = requireMinimal(), f, h = u.LongBits, c = u.base64, d = u.utf8;
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
    return u.Buffer ? function() {
      return (i.create = function() {
        return new f();
      })();
    } : function() {
      return new i();
    };
  };
  i.create = t(), i.alloc = function(v) {
    return new u.Array(v);
  }, u.Array !== Array && (i.alloc = u.pool(i.alloc, u.Array.prototype.subarray)), i.prototype._push = function(v, m, _) {
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
  function a(E, v) {
    this.len = E, this.next = void 0, this.val = v;
  }
  a.prototype = Object.create(n.prototype), a.prototype.fn = s, i.prototype.uint32 = function(v) {
    return this.len += (this.tail = this.tail.next = new a(
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
    return this._push(u.float.writeFloatLE, 4, v);
  }, i.prototype.double = function(v) {
    return this._push(u.float.writeDoubleLE, 8, v);
  };
  var y = u.Array.prototype.set ? function(v, m, _) {
    m.set(v, _);
  } : function(v, m, _) {
    for (var b = 0; b < v.length; ++b)
      m[_ + b] = v[b];
  };
  return i.prototype.bytes = function(v) {
    var m = v.length >>> 0;
    if (!m)
      return this._push(l, 1, 0);
    if (u.isString(v)) {
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
  var u = requireWriter();
  (h.prototype = Object.create(u.prototype)).constructor = h;
  var f = requireMinimal();
  function h() {
    u.call(this);
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
  var u = requireMinimal(), f, h = u.LongBits, c = u.utf8;
  function d(s, a) {
    return RangeError("index out of range: " + s.pos + " + " + (a || 1) + " > " + s.len);
  }
  function n(s) {
    this.buf = s, this.pos = 0, this.len = s.length;
  }
  var e = typeof Uint8Array < "u" ? function(a) {
    if (a instanceof Uint8Array || Array.isArray(a))
      return new n(a);
    throw Error("illegal buffer");
  } : function(a) {
    if (Array.isArray(a))
      return new n(a);
    throw Error("illegal buffer");
  }, r = function() {
    return u.Buffer ? function(o) {
      return (n.create = function(y) {
        return u.Buffer.isBuffer(y) ? new f(y) : e(y);
      })(o);
    } : e;
  };
  n.create = r(), n.prototype._slice = u.Array.prototype.subarray || /* istanbul ignore next */
  u.Array.prototype.slice, n.prototype.uint32 = /* @__PURE__ */ function() {
    var a = 4294967295;
    return function() {
      if (a = (this.buf[this.pos] & 127) >>> 0, this.buf[this.pos++] < 128 || (a = (a | (this.buf[this.pos] & 127) << 7) >>> 0, this.buf[this.pos++] < 128) || (a = (a | (this.buf[this.pos] & 127) << 14) >>> 0, this.buf[this.pos++] < 128) || (a = (a | (this.buf[this.pos] & 127) << 21) >>> 0, this.buf[this.pos++] < 128) || (a = (a | (this.buf[this.pos] & 15) << 28) >>> 0, this.buf[this.pos++] < 128)) return a;
      if ((this.pos += 5) > this.len)
        throw this.pos = this.len, d(this, 10);
      return a;
    };
  }(), n.prototype.int32 = function() {
    return this.uint32() | 0;
  }, n.prototype.sint32 = function() {
    var a = this.uint32();
    return a >>> 1 ^ -(a & 1) | 0;
  };
  function i() {
    var s = new h(0, 0), a = 0;
    if (this.len - this.pos > 4) {
      for (; a < 4; ++a)
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << a * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      if (s.lo = (s.lo | (this.buf[this.pos] & 127) << 28) >>> 0, s.hi = (s.hi | (this.buf[this.pos] & 127) >> 4) >>> 0, this.buf[this.pos++] < 128)
        return s;
      a = 0;
    } else {
      for (; a < 3; ++a) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.lo = (s.lo | (this.buf[this.pos] & 127) << a * 7) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
      return s.lo = (s.lo | (this.buf[this.pos++] & 127) << a * 7) >>> 0, s;
    }
    if (this.len - this.pos > 4) {
      for (; a < 5; ++a)
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << a * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
    } else
      for (; a < 5; ++a) {
        if (this.pos >= this.len)
          throw d(this);
        if (s.hi = (s.hi | (this.buf[this.pos] & 127) << a * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
          return s;
      }
    throw Error("invalid varint encoding");
  }
  n.prototype.bool = function() {
    return this.uint32() !== 0;
  };
  function t(s, a) {
    return (s[a - 4] | s[a - 3] << 8 | s[a - 2] << 16 | s[a - 1] << 24) >>> 0;
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
    var a = u.float.readFloatLE(this.buf, this.pos);
    return this.pos += 4, a;
  }, n.prototype.double = function() {
    if (this.pos + 8 > this.len)
      throw d(this, 4);
    var a = u.float.readDoubleLE(this.buf, this.pos);
    return this.pos += 8, a;
  }, n.prototype.bytes = function() {
    var a = this.uint32(), o = this.pos, p = this.pos + a;
    if (p > this.len)
      throw d(this, a);
    if (this.pos += a, Array.isArray(this.buf))
      return this.buf.slice(o, p);
    if (o === p) {
      var y = u.Buffer;
      return y ? y.alloc(0) : new this.buf.constructor(0);
    }
    return this._slice.call(this.buf, o, p);
  }, n.prototype.string = function() {
    var a = this.bytes();
    return c.read(a, 0, a.length);
  }, n.prototype.skip = function(a) {
    if (typeof a == "number") {
      if (this.pos + a > this.len)
        throw d(this, a);
      this.pos += a;
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
    var a = u.Long ? "toLong" : (
      /* istanbul ignore next */
      "toNumber"
    );
    u.merge(n.prototype, {
      int64: function() {
        return i.call(this)[a](!1);
      },
      uint64: function() {
        return i.call(this)[a](!0);
      },
      sint64: function() {
        return i.call(this).zzDecode()[a](!1);
      },
      fixed64: function() {
        return l.call(this)[a](!0);
      },
      sfixed64: function() {
        return l.call(this)[a](!1);
      }
    });
  }, reader;
}
var reader_buffer, hasRequiredReader_buffer;
function requireReader_buffer() {
  if (hasRequiredReader_buffer) return reader_buffer;
  hasRequiredReader_buffer = 1, reader_buffer = h;
  var u = requireReader();
  (h.prototype = Object.create(u.prototype)).constructor = h;
  var f = requireMinimal();
  function h(c) {
    u.call(this, c);
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
  var u = requireMinimal();
  (f.prototype = Object.create(u.EventEmitter.prototype)).constructor = f;
  function f(h, c, d) {
    if (typeof h != "function")
      throw TypeError("rpcImpl must be a function");
    u.EventEmitter.call(this), this.rpcImpl = h, this.requestDelimited = !!c, this.responseDelimited = !!d;
  }
  return f.prototype.rpcCall = function h(c, d, n, e, r) {
    if (!e)
      throw TypeError("request must be specified");
    var i = this;
    if (!r)
      return u.asPromise(h, i, c, d, n, e);
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
            } catch (a) {
              return i.emit("error", a, c), r(a);
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
  return hasRequiredRpc || (hasRequiredRpc = 1, function(u) {
    var f = u;
    f.Service = requireService$1();
  }(rpc)), rpc;
}
var roots, hasRequiredRoots;
function requireRoots() {
  return hasRequiredRoots || (hasRequiredRoots = 1, roots = {}), roots;
}
var hasRequiredIndexMinimal;
function requireIndexMinimal() {
  return hasRequiredIndexMinimal || (hasRequiredIndexMinimal = 1, function(u) {
    var f = u;
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
  hasRequiredCodegen = 1, codegen_1 = u;
  function u(f, h) {
    typeof f == "string" && (h = f, f = void 0);
    var c = [];
    function d(e) {
      if (typeof e != "string") {
        var r = n();
        if (u.verbose && console.log("codegen: " + r), r = "return " + r, e) {
          for (var i = Object.keys(e), t = new Array(i.length + 1), l = new Array(i.length), s = 0; s < i.length; )
            t[s] = i[s], l[s] = e[i[s++]];
          return t[s] = r, Function.apply(null, t).apply(null, l);
        }
        return Function(r)();
      }
      for (var a = new Array(arguments.length - 1), o = 0; o < a.length; )
        a[o] = arguments[++o];
      if (o = 0, e = e.replace(/%([%dfijs])/g, function(y, E) {
        var v = a[o++];
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
      }), o !== a.length)
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
  return u.verbose = !1, codegen_1;
}
var fetch_1, hasRequiredFetch;
function requireFetch() {
  if (hasRequiredFetch) return fetch_1;
  hasRequiredFetch = 1, fetch_1 = c;
  var u = requireAspromise(), f = requireInquire(), h = f("fs");
  function c(d, n, e) {
    return typeof n == "function" ? (e = n, n = {}) : n || (n = {}), e ? !n.xhr && h && h.readFile ? h.readFile(d, function(i, t) {
      return i && typeof XMLHttpRequest < "u" ? c.xhr(d, n, e) : i ? e(i) : e(null, n.binary ? t : t.toString("utf8"));
    }) : c.xhr(d, n, e) : u(c, this, d, n);
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
  return hasRequiredPath || (hasRequiredPath = 1, function(u) {
    var f = u, h = (
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
  var u = requireObject();
  ((i.prototype = Object.create(u.prototype)).constructor = i).className = "Namespace";
  var f = requireField(), h = requireUtil(), c = requireOneof(), d, n, e;
  i.fromJSON = function(s, a) {
    return new i(s, a.options).addJSON(a.nested);
  };
  function r(l, s) {
    if (l && l.length) {
      for (var a = {}, o = 0; o < l.length; ++o)
        a[l[o].name] = l[o].toJSON(s);
      return a;
    }
  }
  i.arrayToJSON = r, i.isReservedId = function(s, a) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (typeof s[o] != "string" && s[o][0] <= a && s[o][1] > a)
          return !0;
    }
    return !1;
  }, i.isReservedName = function(s, a) {
    if (s) {
      for (var o = 0; o < s.length; ++o)
        if (s[o] === a)
          return !0;
    }
    return !1;
  };
  function i(l, s) {
    u.call(this, l, s), this.nested = void 0, this._nestedArray = null, this._lookupCache = {}, this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
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
    var a = this;
    if (s)
      for (var o = Object.keys(s), p = 0, y; p < o.length; ++p)
        y = s[o[p]], a.add(
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
      var a = this.get(s.name);
      if (a)
        if (a instanceof i && s instanceof i && !(a instanceof d || a instanceof n)) {
          for (var o = a.nestedArray, p = 0; p < o.length; ++p)
            s.add(o[p]);
          this.remove(a), this.nested || (this.nested = {}), s.setOptions(a.options, !0);
        } else
          throw Error("duplicate name '" + s.name + "' in " + this);
    }
    this.nested[s.name] = s, this instanceof d || this instanceof n || this instanceof e || this instanceof f || s._edition || (s._edition = s._defaultEdition), this._needsRecursiveFeatureResolution = !0, this._needsRecursiveResolve = !0;
    for (var y = this; y = y.parent; )
      y._needsRecursiveFeatureResolution = !0, y._needsRecursiveResolve = !0;
    return s.onAdd(this), t(this);
  }, i.prototype.remove = function(s) {
    if (!(s instanceof u))
      throw TypeError("object must be a ReflectionObject");
    if (s.parent !== this)
      throw Error(s + " is not a member of " + this);
    return delete this.nested[s.name], Object.keys(this.nested).length || (this.nested = void 0), s.onRemove(this), t(this);
  }, i.prototype.define = function(s, a) {
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
    return a && o.addJSON(a), o;
  }, i.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    this._resolveFeaturesRecursive(this._edition);
    var s = this.nestedArray, a = 0;
    for (this.resolve(); a < s.length; )
      s[a] instanceof i ? s[a++].resolveAll() : s[a++].resolve();
    return this._needsRecursiveResolve = !1, this;
  }, i.prototype._resolveFeaturesRecursive = function(s) {
    return this._needsRecursiveFeatureResolution ? (this._needsRecursiveFeatureResolution = !1, s = this._edition || s, u.prototype._resolveFeaturesRecursive.call(this, s), this.nestedArray.forEach((a) => {
      a._resolveFeaturesRecursive(s);
    }), this) : this;
  }, i.prototype.lookup = function(s, a, o) {
    if (typeof a == "boolean" ? (o = a, a = void 0) : a && !Array.isArray(a) && (a = [a]), h.isString(s) && s.length) {
      if (s === ".")
        return this.root;
      s = s.split(".");
    } else if (!s.length)
      return this;
    var p = s.join(".");
    if (s[0] === "")
      return this.root.lookup(s.slice(1), a);
    var y = this.root._fullyQualifiedObjects && this.root._fullyQualifiedObjects["." + p];
    if (y && (!a || a.indexOf(y.constructor) > -1) || (y = this._lookupImpl(s, p), y && (!a || a.indexOf(y.constructor) > -1)))
      return y;
    if (o)
      return null;
    for (var E = this; E.parent; ) {
      if (y = E.parent._lookupImpl(s, p), y && (!a || a.indexOf(y.constructor) > -1))
        return y;
      E = E.parent;
    }
    return null;
  }, i.prototype._lookupImpl = function(s, a) {
    if (Object.prototype.hasOwnProperty.call(this._lookupCache, a))
      return this._lookupCache[a];
    var o = this.get(s[0]), p = null;
    if (o)
      s.length === 1 ? p = o : o instanceof i && (s = s.slice(1), p = o._lookupImpl(s, s.join(".")));
    else
      for (var y = 0; y < this.nestedArray.length; ++y)
        this._nestedArray[y] instanceof i && (o = this._nestedArray[y]._lookupImpl(s, a)) && (p = o);
    return this._lookupCache[a] = p, p;
  }, i.prototype.lookupType = function(s) {
    var a = this.lookup(s, [d]);
    if (!a)
      throw Error("no such type: " + s);
    return a;
  }, i.prototype.lookupEnum = function(s) {
    var a = this.lookup(s, [e]);
    if (!a)
      throw Error("no such Enum '" + s + "' in " + this);
    return a;
  }, i.prototype.lookupTypeOrEnum = function(s) {
    var a = this.lookup(s, [d, e]);
    if (!a)
      throw Error("no such Type or Enum '" + s + "' in " + this);
    return a;
  }, i.prototype.lookupService = function(s) {
    var a = this.lookup(s, [n]);
    if (!a)
      throw Error("no such Service '" + s + "' in " + this);
    return a;
  }, i._configure = function(l, s, a) {
    d = l, n = s, e = a;
  }, namespace;
}
var mapfield, hasRequiredMapfield;
function requireMapfield() {
  if (hasRequiredMapfield) return mapfield;
  hasRequiredMapfield = 1, mapfield = c;
  var u = requireField();
  ((c.prototype = Object.create(u.prototype)).constructor = c).className = "MapField";
  var f = requireTypes(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (u.call(this, d, n, r, void 0, void 0, i, t), !h.isString(e))
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
    return u.prototype.resolve.call(this);
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
  var u = requireObject();
  ((h.prototype = Object.create(u.prototype)).constructor = h).className = "Method";
  var f = requireUtil();
  function h(c, d, n, e, r, i, t, l, s) {
    if (f.isObject(r) ? (t = r, r = i = void 0) : f.isObject(i) && (t = i, i = void 0), !(d === void 0 || f.isString(d)))
      throw TypeError("type must be a string");
    if (!f.isString(n))
      throw TypeError("requestType must be a string");
    if (!f.isString(e))
      throw TypeError("responseType must be a string");
    u.call(this, c, t), this.type = d || "rpc", this.requestType = n, this.requestStream = r ? !0 : void 0, this.responseType = e, this.responseStream = i ? !0 : void 0, this.resolvedRequestType = null, this.resolvedResponseType = null, this.comment = l, this.parsedOptions = s;
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
    return this.resolved ? this : (this.resolvedRequestType = this.parent.lookupType(this.requestType), this.resolvedResponseType = this.parent.lookupType(this.responseType), u.prototype.resolve.call(this));
  }, method;
}
var service, hasRequiredService;
function requireService() {
  if (hasRequiredService) return service;
  hasRequiredService = 1, service = d;
  var u = requireNamespace();
  ((d.prototype = Object.create(u.prototype)).constructor = d).className = "Service";
  var f = requireMethod(), h = requireUtil(), c = requireRpc();
  function d(e, r) {
    u.call(this, e, r), this.methods = {}, this._methodsArray = null;
  }
  d.fromJSON = function(r, i) {
    var t = new d(r, i.options);
    if (i.methods)
      for (var l = Object.keys(i.methods), s = 0; s < l.length; ++s)
        t.add(f.fromJSON(l[s], i.methods[l[s]]));
    return i.nested && t.addJSON(i.nested), i.edition && (t._edition = i.edition), t.comment = i.comment, t._defaultEdition = "proto3", t;
  }, d.prototype.toJSON = function(r) {
    var i = u.prototype.toJSON.call(this, r), t = r ? !!r.keepComments : !1;
    return h.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      i && i.options || void 0,
      "methods",
      u.arrayToJSON(this.methodsArray, r) || /* istanbul ignore next */
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
    return this.methods[r] || u.prototype.get.call(this, r);
  }, d.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    u.prototype.resolve.call(this);
    for (var r = this.methodsArray, i = 0; i < r.length; ++i)
      r[i].resolve();
    return this;
  }, d.prototype._resolveFeaturesRecursive = function(r) {
    return this._needsRecursiveFeatureResolution ? (r = this._edition || r, u.prototype._resolveFeaturesRecursive.call(this, r), this.methodsArray.forEach((i) => {
      i._resolveFeaturesRecursive(r);
    }), this) : this;
  }, d.prototype.add = function(r) {
    if (this.get(r.name))
      throw Error("duplicate name '" + r.name + "' in " + this);
    return r instanceof f ? (this.methods[r.name] = r, r.parent = this, n(this)) : u.prototype.add.call(this, r);
  }, d.prototype.remove = function(r) {
    if (r instanceof f) {
      if (this.methods[r.name] !== r)
        throw Error(r + " is not a member of " + this);
      return delete this.methods[r.name], r.parent = null, n(this);
    }
    return u.prototype.remove.call(this, r);
  }, d.prototype.create = function(r, i, t) {
    for (var l = new c.Service(r, i, t), s = 0, a; s < /* initializes */
    this.methodsArray.length; ++s) {
      var o = h.lcFirst((a = this._methodsArray[s]).resolve().name).replace(/[^$\\w_]/g, "");
      l[o] = h.codegen(["r", "c"], h.isReserved(o) ? o + "_" : o)("return this.rpcCall(m,q,s,r,c)")({
        m: a,
        q: a.resolvedRequestType.ctor,
        s: a.resolvedResponseType.ctor
      });
    }
    return l;
  }, service;
}
var message, hasRequiredMessage;
function requireMessage() {
  if (hasRequiredMessage) return message;
  hasRequiredMessage = 1, message = f;
  var u = requireMinimal();
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
    return this.$type.toObject(this, u.toJSONOptions);
  }, message;
}
var decoder_1, hasRequiredDecoder;
function requireDecoder() {
  if (hasRequiredDecoder) return decoder_1;
  hasRequiredDecoder = 1, decoder_1 = d;
  var u = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n) {
    return "missing required '" + n.name + "'";
  }
  function d(n) {
    for (var e = h.codegen(["r", "l", "e"], n.name + "$decode")("if(!(r instanceof Reader))")("r=Reader.create(r)")("var c=l===undefined?r.len:r.pos+l,m=new this.ctor" + (n.fieldsArray.filter(function(a) {
      return a.map;
    }).length ? ",k,value" : ""))("while(r.pos<c){")("var t=r.uint32()")("if(t===e)")("break")("switch(t>>>3){"), r = 0; r < /* initializes */
    n.fieldsArray.length; ++r) {
      var i = n._fieldsArray[r].resolve(), t = i.resolvedType instanceof u ? "int32" : i.type, l = "m" + h.safeProp(i.name);
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
  var u = require_enum(), f = requireUtil();
  function h(e, r) {
    return e.name + ": " + r + (e.repeated && r !== "array" ? "[]" : e.map && r !== "object" ? "{k:" + e.keyType + "}" : "") + " expected";
  }
  function c(e, r, i, t) {
    if (r.resolvedType)
      if (r.resolvedType instanceof u) {
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
      var s = e._fieldsArray[l].resolve(), a = "m" + f.safeProp(s.name);
      if (s.optional && r("if(%s!=null&&m.hasOwnProperty(%j)){", a, s.name), s.map)
        r("if(!util.isObject(%s))", a)("return%j", h(s, "object"))("var k=Object.keys(%s)", a)("for(var i=0;i<k.length;++i){"), d(r, s, "k[i]"), c(r, s, l, a + "[k[i]]")("}");
      else if (s.repeated)
        r("if(!Array.isArray(%s))", a)("return%j", h(s, "array"))("for(var i=0;i<%s.length;++i){", a), c(r, s, l, a + "[i]")("}");
      else {
        if (s.partOf) {
          var o = f.safeProp(s.partOf.name);
          t[s.partOf.name] === 1 && r("if(p%s===1)", o)("return%j", s.partOf.name + ": multiple values"), t[s.partOf.name] = 1, r("p%s=1", o);
        }
        c(r, s, l, a);
      }
      s.optional && r("}");
    }
    return r("return null");
  }
  return verifier_1;
}
var converter = {}, hasRequiredConverter;
function requireConverter() {
  return hasRequiredConverter || (hasRequiredConverter = 1, function(u) {
    var f = u, h = require_enum(), c = requireUtil();
    function d(e, r, i, t) {
      var l = !1;
      if (r.resolvedType)
        if (r.resolvedType instanceof h) {
          e("switch(d%s){", t);
          for (var s = r.resolvedType.values, a = Object.keys(s), o = 0; o < a.length; ++o)
            s[a[o]] === r.typeDefault && !l && (e("default:")('if(typeof(d%s)==="number"){m%s=d%s;break}', t, t, t), r.repeated || e("break"), l = !0), e("case%j:", a[o])("case %i:", s[a[o]])("m%s=%j", t, s[a[o]])("break");
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
        var s = i[l].resolve(), a = c.safeProp(s.name);
        s.map ? (t("if(d%s){", a)('if(typeof d%s!=="object")', a)("throw TypeError(%j)", s.fullName + ": object expected")("m%s={}", a)("for(var ks=Object.keys(d%s),i=0;i<ks.length;++i){", a), d(
          t,
          s,
          /* not sorted */
          l,
          a + "[ks[i]]"
        )("}")("}")) : s.repeated ? (t("if(d%s){", a)("if(!Array.isArray(d%s))", a)("throw TypeError(%j)", s.fullName + ": array expected")("m%s=[]", a)("for(var i=0;i<d%s.length;++i){", a), d(
          t,
          s,
          /* not sorted */
          l,
          a + "[i]"
        )("}")("}")) : (s.resolvedType instanceof h || t("if(d%s!=null){", a), d(
          t,
          s,
          /* not sorted */
          l,
          a
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
      for (var t = c.codegen(["m", "o"], r.name + "$toObject")("if(!o)")("o={}")("var d={}"), l = [], s = [], a = [], o = 0; o < i.length; ++o)
        i[o].partOf || (i[o].resolve().repeated ? l : i[o].map ? s : a).push(i[o]);
      if (l.length) {
        for (t("if(o.arrays||o.defaults){"), o = 0; o < l.length; ++o) t("d%s=[]", c.safeProp(l[o].name));
        t("}");
      }
      if (s.length) {
        for (t("if(o.objects||o.defaults){"), o = 0; o < s.length; ++o) t("d%s={}", c.safeProp(s[o].name));
        t("}");
      }
      if (a.length) {
        for (t("if(o.defaults){"), o = 0; o < a.length; ++o) {
          var p = a[o], y = c.safeProp(p.name);
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
  return hasRequiredWrappers || (hasRequiredWrappers = 1, function(u) {
    var f = u, h = requireMessage();
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
  var u = requireNamespace();
  ((y.prototype = Object.create(u.prototype)).constructor = y).className = "Type";
  var f = require_enum(), h = requireOneof(), c = requireField(), d = requireMapfield(), n = requireService(), e = requireMessage(), r = requireReader(), i = requireWriter(), t = requireUtil(), l = requireEncoder(), s = requireDecoder(), a = requireVerifier(), o = requireConverter(), p = requireWrappers();
  function y(v, m) {
    v = v.replace(/\\W/g, ""), u.call(this, v, m), this.fields = {}, this.oneofs = void 0, this.extensions = void 0, this.reserved = void 0, this.group = void 0, this._fieldsById = null, this._fieldsArray = null, this._oneofsArray = null, this._ctor = null;
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
          var _ = this.fields[v[m]], b = _.id;
          if (this._fieldsById[b])
            throw Error("duplicate id " + b + " in " + this);
          this._fieldsById[b] = _;
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
        var b = {};
        for (_ = 0; _ < /* initializes */
        this.oneofsArray.length; ++_)
          b[this._oneofsArray[_].resolve().name] = {
            get: t.oneOfGetter(this._oneofsArray[_].oneof),
            set: t.oneOfSetter(this._oneofsArray[_].oneof)
          };
        _ && Object.defineProperties(v.prototype, b);
      }
    }
  }), y.generateConstructor = function(m) {
    for (var _ = t.codegen(["p"], m.name), b = 0, I; b < m.fieldsArray.length; ++b)
      (I = m._fieldsArray[b]).map ? _("this%s={}", t.safeProp(I.name)) : I.repeated && _("this%s=[]", t.safeProp(I.name));
    return _("if(p)for(var ks=Object.keys(p),i=0;i<ks.length;++i)if(p[ks[i]]!=null)")("this[ks[i]]=p[ks[i]]");
  };
  function E(v) {
    return v._fieldsById = v._fieldsArray = v._oneofsArray = null, delete v.encode, delete v.decode, delete v.verify, v;
  }
  return y.fromJSON = function(m, _) {
    var b = new y(m, _.options);
    b.extensions = _.extensions, b.reserved = _.reserved;
    for (var I = Object.keys(_.fields), C = 0; C < I.length; ++C)
      b.add(
        (typeof _.fields[I[C]].keyType < "u" ? d.fromJSON : c.fromJSON)(I[C], _.fields[I[C]])
      );
    if (_.oneofs)
      for (I = Object.keys(_.oneofs), C = 0; C < I.length; ++C)
        b.add(h.fromJSON(I[C], _.oneofs[I[C]]));
    if (_.nested)
      for (I = Object.keys(_.nested), C = 0; C < I.length; ++C) {
        var j = _.nested[I[C]];
        b.add(
          // most to least likely
          (j.id !== void 0 ? c.fromJSON : j.fields !== void 0 ? y.fromJSON : j.values !== void 0 ? f.fromJSON : j.methods !== void 0 ? n.fromJSON : u.fromJSON)(I[C], j)
        );
      }
    return _.extensions && _.extensions.length && (b.extensions = _.extensions), _.reserved && _.reserved.length && (b.reserved = _.reserved), _.group && (b.group = !0), _.comment && (b.comment = _.comment), _.edition && (b._edition = _.edition), b._defaultEdition = "proto3", b;
  }, y.prototype.toJSON = function(m) {
    var _ = u.prototype.toJSON.call(this, m), b = m ? !!m.keepComments : !1;
    return t.toObject([
      "edition",
      this._editionToJSON(),
      "options",
      _ && _.options || void 0,
      "oneofs",
      u.arrayToJSON(this.oneofsArray, m),
      "fields",
      u.arrayToJSON(this.fieldsArray.filter(function(I) {
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
      b ? this.comment : void 0
    ]);
  }, y.prototype.resolveAll = function() {
    if (!this._needsRecursiveResolve) return this;
    u.prototype.resolveAll.call(this);
    var m = this.oneofsArray;
    for (b = 0; b < m.length; )
      m[b++].resolve();
    for (var _ = this.fieldsArray, b = 0; b < _.length; )
      _[b++].resolve();
    return this;
  }, y.prototype._resolveFeaturesRecursive = function(m) {
    return this._needsRecursiveFeatureResolution ? (m = this._edition || m, u.prototype._resolveFeaturesRecursive.call(this, m), this.oneofsArray.forEach((_) => {
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
    return m instanceof h ? (this.oneofs || (this.oneofs = {}), this.oneofs[m.name] = m, m.onAdd(this), E(this)) : u.prototype.add.call(this, m);
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
    return u.prototype.remove.call(this, m);
  }, y.prototype.isReservedId = function(m) {
    return u.isReservedId(this.reserved, m);
  }, y.prototype.isReservedName = function(m) {
    return u.isReservedName(this.reserved, m);
  }, y.prototype.create = function(m) {
    return new this.ctor(m);
  }, y.prototype.setup = function() {
    for (var m = this.fullName, _ = [], b = 0; b < /* initializes */
    this.fieldsArray.length; ++b)
      _.push(this._fieldsArray[b].resolve().resolvedType);
    this.encode = l(this)({
      Writer: i,
      types: _,
      util: t
    }), this.decode = s(this)({
      Reader: r,
      types: _,
      util: t
    }), this.verify = a(this)({
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
    return function(b) {
      t.decorateType(b, m);
    };
  }, type;
}
var root$1, hasRequiredRoot;
function requireRoot() {
  if (hasRequiredRoot) return root$1;
  hasRequiredRoot = 1, root$1 = i;
  var u = requireNamespace();
  ((i.prototype = Object.create(u.prototype)).constructor = i).className = "Root";
  var f = requireField(), h = require_enum(), c = requireOneof(), d = requireUtil(), n, e, r;
  function i(a) {
    u.call(this, "", a), this.deferred = [], this.files = [], this._edition = "proto2", this._fullyQualifiedObjects = {};
  }
  i.fromJSON = function(o, p) {
    return p || (p = new i()), o.options && p.setOptions(o.options), p.addJSON(o.nested).resolveAll();
  }, i.prototype.resolvePath = d.path.resolve, i.prototype.fetch = d.fetch;
  function t() {
  }
  i.prototype.load = function a(o, p, y) {
    typeof p == "function" && (y = p, p = void 0);
    var E = this;
    if (!y)
      return d.asPromise(a, E, o, p);
    var v = y === t;
    function m(D, P) {
      if (y) {
        if (v)
          throw D;
        P && P.resolveAll();
        var S = y;
        y = null, S(D, P);
      }
    }
    function _(D) {
      var P = D.lastIndexOf("google/protobuf/");
      if (P > -1) {
        var S = D.substring(P);
        if (S in r) return S;
      }
      return null;
    }
    function b(D, P) {
      try {
        if (d.isString(P) && P.charAt(0) === "{" && (P = JSON.parse(P)), !d.isString(P))
          E.setOptions(P.options).addJSON(P.nested);
        else {
          e.filename = D;
          var S = e(P, E, p), J, U = 0;
          if (S.imports)
            for (; U < S.imports.length; ++U)
              (J = _(S.imports[U]) || E.resolvePath(D, S.imports[U])) && I(J);
          if (S.weakImports)
            for (U = 0; U < S.weakImports.length; ++U)
              (J = _(S.weakImports[U]) || E.resolvePath(D, S.weakImports[U])) && I(J, !0);
        }
      } catch (T) {
        m(T);
      }
      !v && !C && m(null, E);
    }
    function I(D, P) {
      if (D = _(D) || D, !(E.files.indexOf(D) > -1)) {
        if (E.files.push(D), D in r) {
          v ? b(D, r[D]) : (++C, setTimeout(function() {
            --C, b(D, r[D]);
          }));
          return;
        }
        if (v) {
          var S;
          try {
            S = d.fs.readFileSync(D).toString("utf8");
          } catch (J) {
            P || m(J);
            return;
          }
          b(D, S);
        } else
          ++C, E.fetch(D, function(J, U) {
            if (--C, !!y) {
              if (J) {
                P ? C || m(null, E) : m(J);
                return;
              }
              b(D, U);
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
    return u.prototype.resolveAll.call(this);
  };
  var l = /^[A-Z]/;
  function s(a, o) {
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
    else if (o instanceof u) {
      for (var y = 0; y < /* initializes */
      o.nestedArray.length; ++y)
        this._handleRemove(o._nestedArray[y]);
      l.test(o.name) && delete o.parent[o.name];
    }
    delete this._fullyQualifiedObjects[o.fullName];
  }, i._configure = function(a, o, p) {
    n = a, e = o, r = p;
  }, root$1;
}
var hasRequiredUtil;
function requireUtil() {
  if (hasRequiredUtil) return util.exports;
  hasRequiredUtil = 1;
  var u = util.exports = requireMinimal(), f = requireRoots(), h, c;
  u.codegen = requireCodegen(), u.fetch = requireFetch(), u.path = requirePath(), u.fs = u.inquire("fs"), u.toArray = function(t) {
    if (t) {
      for (var l = Object.keys(t), s = new Array(l.length), a = 0; a < l.length; )
        s[a] = t[l[a++]];
      return s;
    }
    return [];
  }, u.toObject = function(t) {
    for (var l = {}, s = 0; s < t.length; ) {
      var a = t[s++], o = t[s++];
      o !== void 0 && (l[a] = o);
    }
    return l;
  };
  var d = /\\\\/g, n = /"/g;
  u.isReserved = function(t) {
    return /^(?:do|if|in|for|let|new|try|var|case|else|enum|eval|false|null|this|true|void|with|break|catch|class|const|super|throw|while|yield|delete|export|import|public|return|static|switch|typeof|default|extends|finally|package|private|continue|debugger|function|arguments|interface|protected|implements|instanceof)$/.test(t);
  }, u.safeProp = function(t) {
    return !/^[$\\w_]+$/.test(t) || u.isReserved(t) ? '["' + t.replace(d, "\\\\\\\\").replace(n, '\\\\"') + '"]' : "." + t;
  }, u.ucFirst = function(t) {
    return t.charAt(0).toUpperCase() + t.substring(1);
  };
  var e = /_([a-z])/g;
  u.camelCase = function(t) {
    return t.substring(0, 1) + t.substring(1).replace(e, function(l, s) {
      return s.toUpperCase();
    });
  }, u.compareFieldsById = function(t, l) {
    return t.id - l.id;
  }, u.decorateType = function(t, l) {
    if (t.$type)
      return l && t.$type.name !== l && (u.decorateRoot.remove(t.$type), t.$type.name = l, u.decorateRoot.add(t.$type)), t.$type;
    h || (h = requireType());
    var s = new h(l || t.name);
    return u.decorateRoot.add(s), s.ctor = t, Object.defineProperty(t, "$type", { value: s, enumerable: !1 }), Object.defineProperty(t.prototype, "$type", { value: s, enumerable: !1 }), s;
  };
  var r = 0;
  return u.decorateEnum = function(t) {
    if (t.$type)
      return t.$type;
    c || (c = require_enum());
    var l = new c("Enum" + r++, t);
    return u.decorateRoot.add(l), Object.defineProperty(t, "$type", { value: l, enumerable: !1 }), l;
  }, u.setProperty = function(t, l, s, a) {
    function o(p, y, E) {
      var v = y.shift();
      if (v === "__proto__" || v === "prototype")
        return p;
      if (y.length > 0)
        p[v] = o(p[v] || {}, y, E);
      else {
        var m = p[v];
        if (m && a)
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
  }, Object.defineProperty(u, "decorateRoot", {
    get: function() {
      return f.decorated || (f.decorated = new (requireRoot())());
    }
  }), util.exports;
}
var hasRequiredTypes;
function requireTypes() {
  return hasRequiredTypes || (hasRequiredTypes = 1, function(u) {
    var f = u, h = requireUtil(), c = [
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
  var u = requireObject();
  ((e.prototype = Object.create(u.prototype)).constructor = e).className = "Field";
  var f = require_enum(), h = requireTypes(), c = requireUtil(), d, n = /^required|optional|repeated$/;
  e.fromJSON = function(i, t) {
    var l = new e(i, t.id, t.type, t.rule, t.extend, t.options, t.comment);
    return t.edition && (l._edition = t.edition), l._defaultEdition = "proto3", l;
  };
  function e(r, i, t, l, s, a, o) {
    if (c.isObject(l) ? (o = s, a = l, l = s = void 0) : c.isObject(s) && (o = a, a = s, s = void 0), u.call(this, r, a), !c.isInteger(i) || i < 0)
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
    return u.prototype.setOption.call(this, i, t, l);
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
    return this.map ? this.defaultValue = c.emptyObject : this.repeated ? this.defaultValue = c.emptyArray : this.defaultValue = this.typeDefault, this.parent instanceof d && (this.parent.ctor.prototype[this.name] = this.defaultValue), u.prototype.resolve.call(this);
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
    return u.prototype._resolveFeatures.call(this, this._edition || i);
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
  var u = requireObject();
  ((c.prototype = Object.create(u.prototype)).constructor = c).className = "OneOf";
  var f = requireField(), h = requireUtil();
  function c(n, e, r, i) {
    if (Array.isArray(e) || (r = e, e = void 0), u.call(this, n, r), !(e === void 0 || Array.isArray(e)))
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
    u.prototype.onAdd.call(this, e);
    for (var r = this, i = 0; i < this.oneof.length; ++i) {
      var t = e.get(this.oneof[i]);
      t && !t.partOf && (t.partOf = r, r.fieldsArray.push(t));
    }
    d(this);
  }, c.prototype.onRemove = function(e) {
    for (var r = 0, i; r < this.fieldsArray.length; ++r)
      (i = this.fieldsArray[r]).parent && i.parent.remove(i);
    u.prototype.onRemove.call(this, e);
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
  const u = requireOneof();
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
      if (this.partOf instanceof u) {
        var a = Object.assign({}, this.partOf._features);
        this._features = Object.assign(a, s || {});
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
    var a = this.parsedOptions;
    if (s) {
      var o = a.find(function(E) {
        return Object.prototype.hasOwnProperty.call(E, t);
      });
      if (o) {
        var p = o[t];
        f.setProperty(p, s, l);
      } else
        o = {}, o[t] = f.setProperty({}, s, l), a.push(o);
    } else {
      var y = {};
      y[t] = l, a.push(y);
    }
    return this;
  }, r.prototype.setOptions = function(t, l) {
    if (t)
      for (var s = Object.keys(t), a = 0; a < s.length; ++a)
        this.setOption(s[a], t[s[a]], l);
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
  var u = requireObject();
  ((c.prototype = Object.create(u.prototype)).constructor = c).className = "Enum";
  var f = requireNamespace(), h = requireUtil();
  function c(d, n, e, r, i, t) {
    if (u.call(this, d, e), n && typeof n != "object")
      throw TypeError("values must be an object");
    if (this.valuesById = {}, this.values = Object.create(this.valuesById), this.comment = r, this.comments = i || {}, this.valuesOptions = t, this._valuesFeatures = {}, this.reserved = void 0, n)
      for (var l = Object.keys(n), s = 0; s < l.length; ++s)
        typeof n[l[s]] == "number" && (this.valuesById[this.values[l[s]] = n[l[s]]] = l[s]);
  }
  return c.prototype._resolveFeatures = function(n) {
    return n = this._edition || n, u.prototype._resolveFeatures.call(this, n), Object.keys(this.values).forEach((e) => {
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
  var u = require_enum(), f = requireTypes(), h = requireUtil();
  function c(n, e, r, i) {
    return e.delimited ? n("types[%i].encode(%s,w.uint32(%i)).uint32(%i)", r, i, (e.id << 3 | 3) >>> 0, (e.id << 3 | 4) >>> 0) : n("types[%i].encode(%s,w.uint32(%i).fork()).ldelim()", r, i, (e.id << 3 | 2) >>> 0);
  }
  function d(n) {
    for (var e = h.codegen(["m", "w"], n.name + "$encode")("if(!w)")("w=Writer.create()"), r, i, t = (
      /* initializes */
      n.fieldsArray.slice().sort(h.compareFieldsById)
    ), r = 0; r < t.length; ++r) {
      var l = t[r].resolve(), s = n._fieldsArray.indexOf(l), a = l.resolvedType instanceof u ? "int32" : l.type, o = f.basic[a];
      i = "m" + h.safeProp(l.name), l.map ? (e("if(%s!=null&&Object.hasOwnProperty.call(m,%j)){", i, l.name)("for(var ks=Object.keys(%s),i=0;i<ks.length;++i){", i)("w.uint32(%i).fork().uint32(%i).%s(ks[i])", (l.id << 3 | 2) >>> 0, 8 | f.mapKey[l.keyType], l.keyType), o === void 0 ? e("types[%i].encode(%s[ks[i]],w.uint32(18).fork()).ldelim().ldelim()", s, i) : e(".uint32(%i).%s(%s[ks[i]]).ldelim()", 16 | o, a, i), e("}")("}")) : l.repeated ? (e("if(%s!=null&&%s.length){", i, i), l.packed && f.packed[a] !== void 0 ? e("w.uint32(%i).fork()", (l.id << 3 | 2) >>> 0)("for(var i=0;i<%s.length;++i)", i)("w.%s(%s[i])", a, i)("w.ldelim()") : (e("for(var i=0;i<%s.length;++i)", i), o === void 0 ? c(e, l, s, i + "[i]") : e("w.uint32(%i).%s(%s[i])", (l.id << 3 | o) >>> 0, a, i)), e("}")) : (l.optional && e("if(%s!=null&&Object.hasOwnProperty.call(m,%j))", i, l.name), o === void 0 ? c(e, l, s, i) : e("w.uint32(%i).%s(%s)", (l.id << 3 | o) >>> 0, a, i));
    }
    return e("return w");
  }
  return encoder_1;
}
var hasRequiredIndexLight;
function requireIndexLight() {
  if (hasRequiredIndexLight) return indexLight.exports;
  hasRequiredIndexLight = 1;
  var u = indexLight.exports = requireIndexMinimal();
  u.build = "light";
  function f(c, d, n) {
    return typeof d == "function" ? (n = d, d = new u.Root()) : d || (d = new u.Root()), d.load(c, n);
  }
  u.load = f;
  function h(c, d) {
    return d || (d = new u.Root()), d.loadSync(c);
  }
  return u.loadSync = h, u.encoder = requireEncoder(), u.decoder = requireDecoder(), u.verifier = requireVerifier(), u.converter = requireConverter(), u.ReflectionObject = requireObject(), u.Namespace = requireNamespace(), u.Root = requireRoot(), u.Enum = require_enum(), u.Type = requireType(), u.Field = requireField(), u.OneOf = requireOneof(), u.MapField = requireMapfield(), u.Service = requireService(), u.Method = requireMethod(), u.Message = requireMessage(), u.wrappers = requireWrappers(), u.types = requireTypes(), u.util = requireUtil(), u.ReflectionObject._configure(u.Root), u.Namespace._configure(u.Type, u.Service, u.Enum), u.Root._configure(u.Type), u.Field._configure(u.Type), indexLight.exports;
}
var tokenize_1, hasRequiredTokenize;
function requireTokenize() {
  if (hasRequiredTokenize) return tokenize_1;
  hasRequiredTokenize = 1, tokenize_1 = l;
  var u = /[\\s{}=;:[\\],'"()<>]/g, f = /(?:"([^"\\\\]*(?:\\\\.[^"\\\\]*)*)")/g, h = /(?:'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)')/g, c = /^ *[*/]+ */, d = /^\\s*\\*?\\/*/, n = /\\n/g, e = /\\s/, r = /\\\\(.?)/g, i = {
    0: "\\0",
    r: "\\r",
    n: \`
\`,
    t: "	"
  };
  function t(s) {
    return s.replace(r, function(a, o) {
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
  function l(s, a) {
    s = s.toString();
    var o = 0, p = s.length, y = 1, E = 0, v = {}, m = [], _ = null;
    function b(k) {
      return Error("illegal " + k + " (line " + y + ")");
    }
    function I() {
      var k = _ === "'" ? h : f;
      k.lastIndex = o - 1;
      var L = k.exec(s);
      if (!L)
        throw b("string");
      return o = k.lastIndex, S(_), _ = null, t(L[1]);
    }
    function C(k) {
      return s.charAt(k);
    }
    function j(k, L, F) {
      var W = {
        type: s.charAt(k++),
        lineEmpty: !1,
        leading: F
      }, H;
      a ? H = 2 : H = 3;
      var B = k - H, $;
      do
        if (--B < 0 || ($ = s.charAt(B)) === \`
\`) {
          W.lineEmpty = !0;
          break;
        }
      while ($ === " " || $ === "	");
      for (var X = s.substring(k, L).split(n), z = 0; z < X.length; ++z)
        X[z] = X[z].replace(a ? d : c, "").trim();
      W.text = X.join(\`
\`).trim(), v[y] = W, E = y;
    }
    function K(k) {
      var L = D(k), F = s.substring(k, L), W = /^\\s*\\/\\//.test(F);
      return W;
    }
    function D(k) {
      for (var L = k; L < p && C(L) !== \`
\`; )
        L++;
      return L;
    }
    function P() {
      if (m.length > 0)
        return m.shift();
      if (_)
        return I();
      var k, L, F, W, H, B = o === 0;
      do {
        if (o === p)
          return null;
        for (k = !1; e.test(F = C(o)); )
          if (F === \`
\` && (B = !0, ++y), ++o === p)
            return null;
        if (C(o) === "/") {
          if (++o === p)
            throw b("comment");
          if (C(o) === "/")
            if (a) {
              if (W = o, H = !1, K(o - 1)) {
                H = !0;
                do
                  if (o = D(o), o === p || (o++, !B))
                    break;
                while (K(o));
              } else
                o = Math.min(p, D(o) + 1);
              H && (j(W, o, B), B = !0), y++, k = !0;
            } else {
              for (H = C(W = o + 1) === "/"; C(++o) !== \`
\`; )
                if (o === p)
                  return null;
              ++o, H && (j(W, o - 1, B), B = !0), ++y, k = !0;
            }
          else if ((F = C(o)) === "*") {
            W = o + 1, H = a || C(W) === "*";
            do {
              if (F === \`
\` && ++y, ++o === p)
                throw b("comment");
              L = F, F = C(o);
            } while (L !== "*" || F !== "/");
            ++o, H && (j(W, o - 2, B), B = !0), k = !0;
          } else
            return "/";
        }
      } while (k);
      var $ = o;
      u.lastIndex = 0;
      var X = u.test(C($++));
      if (!X)
        for (; $ < p && !u.test(C($)); )
          ++$;
      var z = s.substring(o, o = $);
      return (z === '"' || z === "'") && (_ = z), z;
    }
    function S(k) {
      m.push(k);
    }
    function J() {
      if (!m.length) {
        var k = P();
        if (k === null)
          return null;
        S(k);
      }
      return m[0];
    }
    function U(k, L) {
      var F = J(), W = F === k;
      if (W)
        return P(), !0;
      if (!L)
        throw b("token '" + F + "', '" + k + "' expected");
      return !1;
    }
    function T(k) {
      var L = null, F;
      return k === void 0 ? (F = v[y - 1], delete v[y - 1], F && (a || F.type === "*" || F.lineEmpty) && (L = F.leading ? F.text : null)) : (E < k && J(), F = v[k], delete v[k], F && !F.lineEmpty && (a || F.type === "/") && (L = F.leading ? null : F.text)), L;
    }
    return Object.defineProperty({
      next: P,
      peek: J,
      push: S,
      skip: U,
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
  var u = requireTokenize(), f = requireRoot(), h = requireType(), c = requireField(), d = requireMapfield(), n = requireOneof(), e = require_enum(), r = requireService(), i = requireMethod(), t = requireObject(), l = requireTypes(), s = requireUtil(), a = /^[1-9][0-9]*$/, o = /^-?[1-9][0-9]*$/, p = /^0[x][0-9a-fA-F]+$/, y = /^-?0[x][0-9a-fA-F]+$/, E = /^0[0-7]+$/, v = /^-?0[0-7]+$/, m = /^(?![eE])[0-9]*(?:\\.[0-9]*)?(?:[eE][+-]?[0-9]+)?$/, _ = /^[a-zA-Z_][a-zA-Z_0-9]*$/, b = /^(?:\\.?[a-zA-Z_][a-zA-Z_0-9]*)(?:\\.[a-zA-Z_][a-zA-Z_0-9]*)*$/;
  function I(C, j, K) {
    j instanceof f || (K = j, j = new f()), K || (K = I.defaults);
    var D = K.preferTrailingComment || !1, P = u(C, K.alternateCommentMode || !1), S = P.next, J = P.push, U = P.peek, T = P.skip, k = P.cmnt, L = !0, F, W, H, B = "proto2", $ = j, X = [], z = {}, ae = K.keepCase ? function(O) {
      return O;
    } : s.camelCase;
    function he() {
      X.forEach((O) => {
        O._edition = B, Object.keys(z).forEach((g) => {
          O.getOption(g) === void 0 && O.setOption(g, z[g], !0);
        });
      });
    }
    function N(O, g, R) {
      var A = I.filename;
      return R || (I.filename = null), Error("illegal " + (g || "token") + " '" + O + "' (" + (A ? A + ", " : "") + "line " + P.line + ")");
    }
    function ee() {
      var O = [], g;
      do {
        if ((g = S()) !== '"' && g !== "'")
          throw N(g);
        O.push(S()), T(g), g = U();
      } while (g === '"' || g === "'");
      return O.join("");
    }
    function ue(O) {
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
        if (b.test(g))
          return g;
        throw N(g, "value");
      }
    }
    function re(O, g) {
      var R, A;
      do
        if (g && ((R = U()) === '"' || R === "'")) {
          var w = ee();
          if (O.push(w), B >= 2023)
            throw N(w, "id");
        } else
          try {
            O.push([A = te(S()), T("to", !0) ? te(S()) : A]);
          } catch (x) {
            if (g && b.test(R) && B >= 2023)
              O.push(R);
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
    function pe(O, g) {
      var R = 1;
      switch (O.charAt(0) === "-" && (R = -1, O = O.substring(1)), O) {
        case "inf":
        case "INF":
        case "Inf":
          return R * (1 / 0);
        case "nan":
        case "NAN":
        case "Nan":
        case "NaN":
          return NaN;
        case "0":
          return 0;
      }
      if (a.test(O))
        return R * parseInt(O, 10);
      if (p.test(O))
        return R * parseInt(O, 16);
      if (E.test(O))
        return R * parseInt(O, 8);
      if (m.test(O))
        return R * parseFloat(O);
      throw N(O, "number", g);
    }
    function te(O, g) {
      switch (O) {
        case "max":
        case "MAX":
        case "Max":
          return 536870911;
        case "0":
          return 0;
      }
      if (!g && O.charAt(0) === "-")
        throw N(O, "id");
      if (o.test(O))
        return parseInt(O, 10);
      if (y.test(O))
        return parseInt(O, 16);
      if (v.test(O))
        return parseInt(O, 8);
      throw N(O, "id");
    }
    function ye() {
      if (F !== void 0)
        throw N("package");
      if (F = S(), !b.test(F))
        throw N(F, "name");
      $ = $.define(F), T(";");
    }
    function me() {
      var O = U(), g;
      switch (O) {
        case "option":
          if (B < "2024")
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
      O = ee(), T(";"), g.push(O);
    }
    function ve() {
      if (T("="), B = ee(), B < 2023)
        throw N(B, "syntax");
      T(";");
    }
    function ge() {
      if (T("="), B = ee(), !["2023", "2024"].includes(B))
        throw N(B, "edition");
      T(";");
    }
    function ie(O, g) {
      switch (g) {
        case "option":
          return Q(O, g), T(";"), !0;
        case "message":
          return ne(O, g), !0;
        case "enum":
          return de(O, g), !0;
        case "export":
        case "local":
          return B < "2024" || (g = S(), g === "export" || g === "local") || g !== "message" && g !== "enum" ? !1 : ie(O, g);
        case "service":
          return Ae(O, g), !0;
        case "extend":
          return Se(O, g), !0;
      }
      return !1;
    }
    function Z(O, g, R) {
      var A = P.line;
      if (O && (typeof O.comment != "string" && (O.comment = k()), O.filename = I.filename), T("{", !0)) {
        for (var w; (w = S()) !== "}"; )
          g(w);
        T(";", !0);
      } else
        R && R(), T(";"), O && (typeof O.comment != "string" || D) && (O.comment = k(A) || O.comment);
    }
    function ne(O, g) {
      if (!_.test(g = S()))
        throw N(g, "type name");
      var R = new h(g);
      Z(R, function(w) {
        if (!ie(R, w))
          switch (w) {
            case "map":
              Ee(R);
              break;
            case "required":
              if (B !== "proto2")
                throw N(w);
            /* eslint-disable no-fallthrough */
            case "repeated":
              Y(R, w);
              break;
            case "optional":
              if (B === "proto3")
                Y(R, "proto3_optional");
              else {
                if (B !== "proto2")
                  throw N(w);
                Y(R, "optional");
              }
              break;
            case "oneof":
              Oe(R, w);
              break;
            case "extensions":
              re(R.extensions || (R.extensions = []));
              break;
            case "reserved":
              re(R.reserved || (R.reserved = []), !0);
              break;
            default:
              if (B === "proto2" || !b.test(w))
                throw N(w);
              J(w), Y(R, "optional");
              break;
          }
      }), O.add(R), O === $ && X.push(R);
    }
    function Y(O, g, R) {
      var A = S();
      if (A === "group") {
        _e(O, g);
        return;
      }
      for (; A.endsWith(".") || U().startsWith("."); )
        A += S();
      if (!b.test(A))
        throw N(A, "type");
      var w = S();
      if (!_.test(w))
        throw N(w, "name");
      w = ae(w), T("=");
      var q = new c(w, te(S()), A, g, R);
      if (Z(q, function(M) {
        if (M === "option")
          Q(q, M), T(";");
        else
          throw N(M);
      }, function() {
        se(q);
      }), g === "proto3_optional") {
        var x = new n("_" + w);
        q.setOption("proto3_optional", !0), x.add(q), O.add(x);
      } else
        O.add(q);
      O === $ && X.push(q);
    }
    function _e(O, g) {
      if (B >= 2023)
        throw N("group");
      var R = S();
      if (!_.test(R))
        throw N(R, "name");
      var A = s.lcFirst(R);
      R === A && (R = s.ucFirst(R)), T("=");
      var w = te(S()), q = new h(R);
      q.group = !0;
      var x = new c(A, w, R, g);
      x.filename = I.filename, Z(q, function(M) {
        switch (M) {
          case "option":
            Q(q, M), T(";");
            break;
          case "required":
          case "repeated":
            Y(q, M);
            break;
          case "optional":
            B === "proto3" ? Y(q, "proto3_optional") : Y(q, "optional");
            break;
          case "message":
            ne(q, M);
            break;
          case "enum":
            de(q, M);
            break;
          case "reserved":
            re(q.reserved || (q.reserved = []), !0);
            break;
          case "export":
          case "local":
            if (B < "2024")
              throw N(M);
            switch (M = S(), M) {
              case "message":
                ne(q, M);
                break;
              case "enum":
                ne(q, M);
                break;
              default:
                throw N(M);
            }
            break;
          /* istanbul ignore next */
          default:
            throw N(M);
        }
      }), O.add(q).add(x);
    }
    function Ee(O) {
      T("<");
      var g = S();
      if (l.mapKey[g] === void 0)
        throw N(g, "type");
      T(",");
      var R = S();
      if (!b.test(R))
        throw N(R, "type");
      T(">");
      var A = S();
      if (!_.test(A))
        throw N(A, "name");
      T("=");
      var w = new d(ae(A), te(S()), g, R);
      Z(w, function(x) {
        if (x === "option")
          Q(w, x), T(";");
        else
          throw N(x);
      }, function() {
        se(w);
      }), O.add(w);
    }
    function Oe(O, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var R = new n(ae(g));
      Z(R, function(w) {
        w === "option" ? (Q(R, w), T(";")) : (J(w), Y(R, "optional"));
      }), O.add(R);
    }
    function de(O, g) {
      if (!_.test(g = S()))
        throw N(g, "name");
      var R = new e(g);
      Z(R, function(w) {
        switch (w) {
          case "option":
            Q(R, w), T(";");
            break;
          case "reserved":
            re(R.reserved || (R.reserved = []), !0), R.reserved === void 0 && (R.reserved = []);
            break;
          default:
            Re(R, w);
        }
      }), O.add(R), O === $ && X.push(R);
    }
    function Re(O, g) {
      if (!_.test(g))
        throw N(g, "name");
      T("=");
      var R = te(S(), !0), A = {
        options: void 0
      };
      A.getOption = function(w) {
        return this.options[w];
      }, A.setOption = function(w, q) {
        t.prototype.setOption.call(A, w, q);
      }, A.setParsedOption = function() {
      }, Z(A, function(q) {
        if (q === "option")
          Q(A, q), T(";");
        else
          throw N(q);
      }, function() {
        se(A);
      }), O.add(g, R, A.comment, A.parsedOptions || A.options);
    }
    function Q(O, g) {
      var R, A, w = !0;
      for (g === "option" && (g = S()); g !== "="; ) {
        if (g === "(") {
          var q = S();
          T(")"), g = "(" + q + ")";
        }
        if (w) {
          if (w = !1, g.includes(".") && !g.includes("(")) {
            var x = g.split(".");
            R = x[0] + ".", g = x[1];
            continue;
          }
          R = g;
        } else
          A = A ? A += g : g;
        g = S();
      }
      var G = A ? R.concat(A) : R, M = ce(O, G);
      A = A && A[0] === "." ? A.slice(1) : A, R = R && R[R.length - 1] === "." ? R.slice(0, -1) : R, be(O, R, M, A);
    }
    function ce(O, g) {
      if (T("{", !0)) {
        for (var R = {}; !T("}", !0); ) {
          if (!_.test(V = S()))
            throw N(V, "name");
          if (V === null)
            throw N(V, "end of input");
          var A, w = V;
          if (T(":", !0), U() === "{")
            A = ce(O, g + "." + V);
          else if (U() === "[") {
            A = [];
            var q;
            if (T("[", !0)) {
              do
                q = ue(), A.push(q);
              while (T(",", !0));
              T("]"), typeof q < "u" && fe(O, g + "." + V, q);
            }
          } else
            A = ue(), fe(O, g + "." + V, A);
          var x = R[w];
          x && (A = [].concat(x).concat(A)), R[w] = A, T(",", !0), T(";", !0);
        }
        return R;
      }
      var G = ue();
      return fe(O, g, G), G;
    }
    function fe(O, g, R) {
      if ($ === O && /^features\\./.test(g)) {
        z[g] = R;
        return;
      }
      O.setOption && O.setOption(g, R);
    }
    function be(O, g, R, A) {
      O.setParsedOption && O.setParsedOption(g, R, A);
    }
    function se(O) {
      if (T("[", !0)) {
        do
          Q(O, "option");
        while (T(",", !0));
        T("]");
      }
      return O;
    }
    function Ae(O, g) {
      if (!_.test(g = S()))
        throw N(g, "service name");
      var R = new r(g);
      Z(R, function(w) {
        if (!ie(R, w))
          if (w === "rpc")
            we(R, w);
          else
            throw N(w);
      }), O.add(R), O === $ && X.push(R);
    }
    function we(O, g) {
      var R = k(), A = g;
      if (!_.test(g = S()))
        throw N(g, "name");
      var w = g, q, x, G, M;
      if (T("("), T("stream", !0) && (x = !0), !b.test(g = S()) || (q = g, T(")"), T("returns"), T("("), T("stream", !0) && (M = !0), !b.test(g = S())))
        throw N(g);
      G = g, T(")");
      var oe = new i(w, A, q, G, x, M);
      oe.comment = R, Z(oe, function(le) {
        if (le === "option")
          Q(oe, le), T(";");
        else
          throw N(le);
      }), O.add(oe);
    }
    function Se(O, g) {
      if (!b.test(g = S()))
        throw N(g, "reference");
      var R = g;
      Z(null, function(w) {
        switch (w) {
          case "required":
          case "repeated":
            Y(O, w, R);
            break;
          case "optional":
            B === "proto3" ? Y(O, "proto3_optional", R) : Y(O, "optional", R);
            break;
          default:
            if (B === "proto2" || !b.test(w))
              throw N(w);
            J(w), Y(O, "optional", R);
            break;
        }
      });
    }
    for (var V; (V = S()) !== null; )
      switch (V) {
        case "package":
          if (!L)
            throw N(V);
          ye();
          break;
        case "import":
          if (!L)
            throw N(V);
          me();
          break;
        case "syntax":
          if (!L)
            throw N(V);
          ve();
          break;
        case "edition":
          if (!L)
            throw N(V);
          ge();
          break;
        case "option":
          Q($, V), T(";", !0);
          break;
        default:
          if (ie($, V)) {
            L = !1;
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
  var u = /\\/|\\./;
  function f(c, d) {
    u.test(c) || (c = "google/protobuf/" + c + ".proto", d = { nested: { google: { nested: { protobuf: { nested: d } } } } }), f[c] = d;
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
  var u = src.exports = requireIndexLight();
  return u.build = "full", u.tokenize = requireTokenize(), u.parse = requireParse(), u.common = requireCommon(), u.Root._configure(u.Type, u.parse, u.common), src.exports;
}
var protobufjs, hasRequiredProtobufjs;
function requireProtobufjs() {
  return hasRequiredProtobufjs || (hasRequiredProtobufjs = 1, protobufjs = requireSrc()), protobufjs;
}
var protobufjsExports = requireProtobufjs(), StateStreamErrorCode = /* @__PURE__ */ ((u) => (u.CONNECTION_FAILED = "CONNECTION_FAILED", u.RECONNECT_FAILED = "RECONNECT_FAILED", u.CONNECTION_LOST = "CONNECTION_LOST", u.CONNECTION_TIMEOUT = "CONNECTION_TIMEOUT", u.AUTH_FAILED = "AUTH_FAILED", u.AUTH_REFRESH_FAILED = "AUTH_REFRESH_FAILED", u.DEVICE_ERROR = "DEVICE_ERROR", u.DECODE_ERROR = "DECODE_ERROR", u.FRAME_PROCESS_ERROR = "FRAME_PROCESS_ERROR", u.STREAM_ALREADY_STARTED = "STREAM_ALREADY_STARTED", u.WORKER_INIT_FAILED = "WORKER_INIT_FAILED", u.UNKNOWN_ERROR = "UNKNOWN_ERROR", u))(StateStreamErrorCode || {}), ConnectionStatus = /* @__PURE__ */ ((u) => (u.DISCONNECTED = "DISCONNECTED", u.CONNECTING = "CONNECTING", u.CONNECTED = "CONNECTED", u.RECONNECTING = "RECONNECTING", u))(ConnectionStatus || {}), AuthStatus = /* @__PURE__ */ ((u) => (u.UNAUTHENTICATED = "UNAUTHENTICATED", u.AUTHENTICATING = "AUTHENTICATING", u.AUTHENTICATED = "AUTHENTICATED", u.FAILED = "FAILED", u))(AuthStatus || {});
const nested = { BSB_State: { nested: { StateUpdate: { oneofs: { state: { oneof: ["deviceName", "power", "brightness", "audioVolume", "wifi", "updateState", "updateCheck", "timezone", "matter", "frame", "input", "timer", "ble", "autoUpdateState"] } }, fields: { deviceName: { type: "BSB_State.DeviceName", id: 1 }, power: { type: "BSB_State.Power", id: 2 }, brightness: { type: "BSB_State.Brightness", id: 3 }, audioVolume: { type: "BSB_State.AudioVolume", id: 4 }, wifi: { type: "BSB_State.Wifi", id: 5 }, updateState: { type: "BSB_Update.UpdateState", id: 6 }, updateCheck: { type: "BSB_Update.CheckState", id: 7 }, timezone: { type: "BSB_State.Timezone", id: 8 }, matter: { type: "BSB_State.Matter", id: 9 }, frame: { type: "BSB_Frame.Frame", id: 10 }, input: { type: "BSB_Input.InputEvent", id: 11 }, timer: { type: "BSB_Timer.Timer", id: 12 }, ble: { type: "BSB_State.Ble.Ble", id: 13 }, autoUpdateState: { type: "BSB_Update.AutoUpdateState", id: 14 } } }, State: { oneofs: { _error: { oneof: ["error"] } }, fields: { timestamp: { type: "fixed64", id: 1 }, updates: { rule: "repeated", type: "StateUpdate", id: 2 }, error: { type: "BSB_Error.Error", id: 3, options: { proto3_optional: !0 } } } }, DeviceName: { fields: { name: { type: "string", id: 1 } } }, BrightnessAutomatic: { fields: {} }, BrightnessManual: { fields: { brightness: { type: "uint32", id: 1 } } }, Brightness: { oneofs: { setting: { oneof: ["automatic", "manual"] } }, fields: { automatic: { type: "BrightnessAutomatic", id: 1 }, manual: { type: "BrightnessManual", id: 2 }, actualBrightness: { type: "uint32", id: 3 } } }, BatteryStatus: { values: { DISCHARGING: 0, CHARGING: 1, CHARGED: 2 } }, UnknownPowerState: { fields: {} }, PowerState: { fields: { batteryStatus: { type: "BatteryStatus", id: 1 }, batteryChargePercent: { type: "uint32", id: 2 }, batteryVoltageMv: { type: "uint32", id: 3 }, batteryCurrentMa: { type: "sint32", id: 4 }, usbVoltageMv: { type: "uint32", id: 5 } } }, Power: { oneofs: { state: { oneof: ["unknown", "known"] } }, fields: { unknown: { type: "UnknownPowerState", id: 1 }, known: { type: "PowerState", id: 2 } } }, AudioVolume: { fields: { volume: { type: "uint32", id: 1 } } }, WifiConnectionStatus: { values: { CONNECTED: 0, CONNECTING: 1, DISCONNECTING: 2, RECONNECTING: 3 } }, WifiSecurity: { values: { UNKNOWN: 0, OPEN: 1, WPA: 2, WPA2: 3, WEP: 4, WPA_WPA2: 5, WPA3: 6, WPA2_WPA3: 7 } }, IpConfigurationMethod: { values: { DHCP: 0, STATIC: 1 } }, IpProtocol: { values: { IPV4: 0, IPV6: 1 } }, WifiStateUnknown: { fields: {} }, WifiStateDisconnected: { fields: {} }, WifiStateConnected: { fields: { status: { type: "WifiConnectionStatus", id: 1 }, ssid: { type: "string", id: 2 }, bssid: { type: "string", id: 3 }, channel: { type: "uint32", id: 4 }, rssi: { type: "sint32", id: 5 }, security: { type: "WifiSecurity", id: 6 } } }, IpAddress: { fields: { protocol: { type: "IpProtocol", id: 1 }, method: { type: "IpConfigurationMethod", id: 2 }, address: { type: "string", id: 3 }, gateway: { type: "string", id: 4 }, netmask: { type: "string", id: 5 } } }, Wifi: { oneofs: { wifiState: { oneof: ["unknown", "disconnected", "connected"] } }, fields: { unknown: { type: "WifiStateUnknown", id: 1 }, disconnected: { type: "WifiStateDisconnected", id: 2 }, connected: { type: "WifiStateConnected", id: 3 }, ipAddresses: { rule: "repeated", type: "IpAddress", id: 4 } } }, Timezone: { fields: { name: { type: "string", id: 1 }, offset: { type: "sint32", id: 2 }, abbr: { type: "string", id: 3 } } }, MatterCommissioningStatus: { values: { NEVER_STARTED: 0, STARTED: 1, COMPLETED_SUCCESSFULLY: 2, FAILED: 3 } }, MatterCommissioningState: { fields: { status: { type: "MatterCommissioningStatus", id: 1 }, timestamp: { type: "fixed64", id: 2 } } }, Matter: { fields: { fabricCount: { type: "uint32", id: 1 }, state: { type: "MatterCommissioningState", id: 2 } } }, Ble: { nested: { ServiceStatus: { values: { RESET: 0, INITIALIZATION: 1, READY: 2, ADVERTISING: 3, CONNECTABLE: 4, CONNECTED: 5, ERROR: 6 } }, Ble: { oneofs: { _remoteAddress: { oneof: ["remoteAddress"] } }, fields: { status: { type: "ServiceStatus", id: 1 }, remoteAddress: { type: "string", id: 2, options: { proto3_optional: !0 } } } } } } } }, BSB_Update: { nested: { UpdateEvent: { values: { SESSION_START: 0, SESSION_STOP: 1, ACTION_BEGIN: 2, ACTION_DONE: 3, DETAIL_CHANGE: 4, ACTION_PROGRESS: 5, EVENT_NONE: 6 } }, UpdateAction: { values: { DOWNLOAD: 0, SHA_VERIFICATION: 1, UNPACK: 2, INSTALLATION_PREPARE: 3, INSTALLATION_APPLY: 4, ACTION_NONE: 5 } }, UpdateStatus: { values: { OK: 0, BATTERY_LOW: 1, BUSY: 2, DOWNLOAD_FAILURE: 3, DOWNLOAD_ABORT: 4, SHA_MISMATCH: 5, UNPACK_CREATE_STAGING_DIRECTORY_FAILURE: 6, UNPACK_ARCHIVE_OPEN_FAILURE: 7, UNPACK_ARCHIVE_UNPACK_FAILURE: 8, INSTALLATION_PREPARE_MANIFEST_NOT_FOUND: 9, INSTALLATION_PREPARE_MANIFEST_INVALID: 10, INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE: 11, INSTALLATION_PREPARE_POINTER_SETUP_FAILURE: 12, UNKNOWN_FAILURE: 13 } }, CheckError: { values: { NOT_AVAILABLE: 0, FAILURE: 1, IDLE: 2 } }, UpdateAvailable: { fields: { version: { type: "string", id: 1 } } }, UpdateUnavailable: { fields: { reason: { type: "CheckError", id: 1 } } }, UpdateState: { fields: { event: { type: "UpdateEvent", id: 1 }, action: { type: "UpdateAction", id: 2 }, status: { type: "UpdateStatus", id: 3 } } }, CheckState: { oneofs: { status: { oneof: ["available", "unavailable"] } }, fields: { available: { type: "UpdateAvailable", id: 1 }, unavailable: { type: "UpdateUnavailable", id: 2 } } }, AutoUpdateInterval: { fields: { start: { type: "uint32", id: 1 }, end: { type: "uint32", id: 2 } } }, AutoUpdateState: { fields: { enabled: { type: "bool", id: 1 }, interval: { type: "AutoUpdateInterval", id: 2 } } } } }, BSB_Frame: { nested: { Encoding: { values: { PLAIN: 0, RUN_LENGTH: 1, DEFLATE: 2, DEFLATE_RUN_LENGTH: 3 } }, PixelFormat: { values: { RGB888: 0, L8: 1, L4: 2 } }, Screen: { values: { FRONT: 0, BACK: 1 } }, Frame: { fields: { screen: { type: "Screen", id: 1 }, width: { type: "uint32", id: 2 }, height: { type: "uint32", id: 3 }, encoding: { type: "Encoding", id: 4 }, pixelFormat: { type: "PixelFormat", id: 5 }, data: { type: "bytes", id: 6 } } } } }, BSB_Timer: { nested: { Timer: { fields: { json: { type: "BSB_Util.Json", id: 1 } } } } }, BSB_Util: { nested: { Compression: { values: { PLAIN: 0, GZIP: 1 } }, Json: { fields: { compression: { type: "Compression", id: 1 }, data: { type: "bytes", id: 2 } } } } }, BSB_Input: { nested: { Button: { values: { OK: 0, BACK: 1, START: 2 } }, ButtonAction: { values: { PRESS: 0, RELEASE: 1 } }, SwitchPosition: { values: { BUSY: 0, CUSTOM: 1, OFF: 2, APPS: 3, SETTINGS: 4 } }, ButtonEvent: { fields: { button: { type: "Button", id: 1 }, action: { type: "ButtonAction", id: 2 } } }, SwitchEvent: { fields: { position: { type: "SwitchPosition", id: 1 } } }, EncoderEvent: { fields: { delta: { type: "sint32", id: 1 } } }, InputEvent: { oneofs: { event: { oneof: ["buttonEvent", "switchEvent", "encoderEvent"] } }, fields: { buttonEvent: { type: "ButtonEvent", id: 1 }, switchEvent: { type: "SwitchEvent", id: 2 }, encoderEvent: { type: "EncoderEvent", id: 3 } } } } }, BSB_Error: { nested: { Cause: { values: { RESOURCE_LIMIT: 0 } }, Severity: { values: { FATAL: 0, ERROR: 1, WARNING: 2 } }, Error: { fields: { cause: { type: "Cause", id: 1 }, severity: { type: "Severity", id: 2 } } } } } };
var bundle = {
  nested
};
function decompressRLE(u, f) {
  const h = [];
  for (let c = 0; c < u.length; ) {
    const d = u[c++];
    if (d === void 0) break;
    const n = d & 127;
    if (!n)
      continue;
    if (d & 128) {
      const r = n * f, i = u.subarray(c, c + r);
      for (let t = 0; t < i.length; t++)
        h.push(i[t]);
      c += r;
      continue;
    }
    const e = u.subarray(c, c + f);
    c += f;
    for (let r = 0; r < n; r++)
      for (let i = 0; i < f; i++)
        h.push(e[i]);
  }
  return new Uint8Array(h);
}
async function decompressDeflate(u) {
  if (typeof DecompressionStream > "u")
    throw new Error("DecompressionStream is not supported in this environment.");
  try {
    const f = new DecompressionStream("deflate"), h = f.writable.getWriter();
    h.write(u), h.close();
    const d = await new Response(f.readable).arrayBuffer();
    return new Uint8Array(d);
  } catch (f) {
    throw new Error(\`Deflate decompression failed: \${f instanceof Error ? f.message : String(f)}\`);
  }
}
function convertL4toRGBA(u, f, h) {
  const c = new Uint8ClampedArray(f * h * 4);
  let d = 0;
  for (let n = 0; n < u.length; n++) {
    const e = u[n], r = (e & 15) * 17, i = (e >> 4 & 15) * 17, t = [r, i];
    for (const l of t)
      if (d < f * h) {
        const s = d * 4;
        c[s] = l, c[s + 1] = l, c[s + 2] = l, c[s + 3] = 255, d++;
      }
  }
  return c;
}
function convertL8toRGBA(u, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = Math.min(u.length, f * h);
  for (let n = 0; n < d; n++) {
    const e = u[n], r = n * 4;
    c[r] = e, c[r + 1] = e, c[r + 2] = e, c[r + 3] = 255;
  }
  return c;
}
function convertRGB888toRGBA(u, f, h) {
  const c = new Uint8ClampedArray(f * h * 4), d = f * h;
  for (let n = 0; n < d; n++) {
    const e = n * 3, r = n * 4;
    e + 2 < u.length && (c[r] = u[e + 2], c[r + 1] = u[e + 1], c[r + 2] = u[e], c[r + 3] = 255);
  }
  return c;
}
async function processFrame(u) {
  if (!u.data || !u.width || !u.height)
    return null;
  let f = u.data;
  const h = u.pixelFormat === 0 ? 3 : 1;
  switch (u.encoding) {
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
  switch (u.pixelFormat) {
    case 2:
      return convertL4toRGBA(f, u.width, u.height);
    case 1:
      return convertL8toRGBA(f, u.width, u.height);
    case 0:
      return convertRGB888toRGBA(f, u.width, u.height);
    default:
      return new Uint8ClampedArray(u.width * u.height * 4);
  }
}
const root = protobufjsExports.Root.fromJSON(bundle), StateType = root.lookupType("BSB_State.State"), AUTH_CODE = 3e3, RECONNECT_CODES = /* @__PURE__ */ new Set([1001, 1006, 1012, 1013, 1014, 3008]), MAX_AUTH_ATTEMPTS = 5, MAX_RECONNECT_ATTEMPTS = 5;
let socket = null, isBinaryMode = !0, currentMode = "local", currentToken, currentAddr = "", retryCount = 0, authRetryCount = 0, isAuthReported = !1, stabilityTimeout;
const activePorts = /* @__PURE__ */ new Set(), subscriptions = /* @__PURE__ */ new Map();
let processingQueue = Promise.resolve();
function broadcast(u) {
  for (const f of activePorts)
    f.postMessage(u);
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
function stopAndCleanup() {
  socket && (socket.close(), socket = null), stabilityTimeout && (clearTimeout(stabilityTimeout), stabilityTimeout = void 0), subscriptions.clear(), activePorts.clear(), retryCount = 0, authRetryCount = 0, isAuthReported = !1;
}
function connect(u, f, h = !0, c = "local") {
  socket && socket.close(), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.CONNECTING }), currentAddr = u, isBinaryMode = h, currentMode = c, currentToken = f, isAuthReported = !1;
  const d = new URL(u);
  socket = new WebSocket(d.toString()), socket.binaryType = "arraybuffer", socket.onopen = () => {
    broadcast({ type: "CONNECTED" }), currentMode === "local" && (socket == null || socket.send(JSON.stringify({ enable: !0 }))), sendAuth(), stabilityTimeout && clearTimeout(stabilityTimeout), stabilityTimeout = setTimeout(() => {
      retryCount = 0, authRetryCount = 0, console.log("[Worker] Connection stable. All retry counters reset.");
    }, 5e3), currentMode === "remote" && subscriptions.size > 0 && sendSubscriptions(), currentMode === "local" && broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED });
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
            const { cause: s, severity: a } = l.error;
            if (s != null && a != null) {
              const o = root.lookupEnum("BSB_Error.Cause"), p = root.lookupEnum("BSB_Error.Severity"), y = o.valuesById[s] || "UNKNOWN", E = p.valuesById[a] || "UNKNOWN";
              if (broadcast({
                type: "ERROR",
                code: StateStreamErrorCode.DEVICE_ERROR,
                message: \`Server reported \${E}: \${y}\`,
                data: l.error
              }), a === p.values.FATAL) {
                stopAndCleanup();
                return;
              }
              if (a === p.values.ERROR)
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
              const a = s.frame;
              if (a && a.data)
                try {
                  const o = await processFrame(a);
                  o && (a.data = o);
                } catch (o) {
                  broadcast({
                    type: "ERROR",
                    code: StateStreamErrorCode.FRAME_PROCESS_ERROR,
                    message: o instanceof Error ? o.message : String(o),
                    data: a.data
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
    if (console.log("[Worker] Socket closed:", n), stabilityTimeout && (clearTimeout(stabilityTimeout), stabilityTimeout = void 0), !socket || activePorts.size === 0) {
      console.log("[Worker] Connection closed or no active ports. No retries.");
      return;
    }
    if (n.code === AUTH_CODE && currentMode === "remote") {
      authRetryCount < MAX_AUTH_ATTEMPTS ? (authRetryCount++, console.warn(\`[Worker] Auth failed (3000). Requesting new token... (Attempt \${authRetryCount}/\${MAX_AUTH_ATTEMPTS})\`), broadcast({ type: "TOKEN_EXPIRED" })) : (broadcast({ type: "STATUS_UPDATE", auth: AuthStatus.FAILED }), broadcast({
        type: "ERROR",
        code: StateStreamErrorCode.AUTH_FAILED,
        message: \`Maximum authentication attempts (\${MAX_AUTH_ATTEMPTS}) reached. Please log in again.\`
      }));
      return;
    }
    if (currentMode === "remote" && RECONNECT_CODES.has(n.code)) {
      if (retryCount < MAX_RECONNECT_ATTEMPTS) {
        retryCount++;
        const e = Math.min(1e3 * retryCount, 5e3);
        console.log(\`[Worker] Reconnecting (network code: \${n.code}) in \${e}ms... (Attempt \${retryCount}/\${MAX_RECONNECT_ATTEMPTS})\`), broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.RECONNECTING }), setTimeout(() => {
          activePorts.size > 0 && socket && connect(currentAddr, currentToken, isBinaryMode, currentMode);
        }, e);
      } else
        broadcast({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), broadcast({
          type: "ERROR",
          code: StateStreamErrorCode.RECONNECT_FAILED,
          message: \`Maximum reconnection attempts (\${MAX_RECONNECT_ATTEMPTS}) reached. Connection lost.\`
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
function handleCommand(u, f) {
  switch (u.type) {
    case "START":
      activePorts.add(f), socket && socket.readyState === WebSocket.OPEN && currentAddr === u.addr ? (f.postMessage({ type: "CONNECTED" }), f.postMessage({ type: "STATUS_UPDATE", auth: AuthStatus.AUTHENTICATED })) : connect(u.addr, u.token, u.isBinary, u.mode);
      break;
    case "STOP":
      activePorts.delete(f);
      for (const [e, r] of subscriptions.entries())
        r.delete(f), r.size === 0 && (subscriptions.delete(e), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [e] })));
      f.postMessage({ type: "STATUS_UPDATE", connection: ConnectionStatus.DISCONNECTED }), activePorts.size === 0 && stopAndCleanup();
      break;
    case "UPDATE_TOKEN":
      const h = currentToken;
      if (currentToken = u.token, currentMode === "remote") {
        const e = socket && socket.readyState === WebSocket.OPEN;
        if (e && h === u.token)
          return;
        e ? sendAuth() : h !== u.token && currentAddr && activePorts.size > 0 && connect(currentAddr, currentToken, isBinaryMode, currentMode);
      }
      break;
    case "SUBSCRIBE":
      let c = subscriptions.get(u.guid);
      c || (c = /* @__PURE__ */ new Set(), subscriptions.set(u.guid, c));
      const d = c.size === 0;
      c.add(f), d && (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ subscribe: [u.guid] }));
      break;
    case "UNSUBSCRIBE":
      const n = subscriptions.get(u.guid);
      n && (n.delete(f), n.size === 0 && (subscriptions.delete(u.guid), (socket == null ? void 0 : socket.readyState) === WebSocket.OPEN && currentMode === "remote" && socket.send(JSON.stringify({ unsubscribe: [u.guid] }))));
      break;
  }
}
if ("SharedWorkerGlobalScope" in self) {
  const u = self;
  u.onconnect = (f) => {
    const h = f.ports[0];
    h && (h.onmessage = (c) => handleCommand(c.data, h), h.start());
  };
} else {
  const u = self;
  u.onmessage = (f) => {
    handleCommand(f.data, u);
  };
}
`;function Mr(t){return new SharedWorker("data:text/javascript;charset=utf-8,"+encodeURIComponent(Fr),{type:"module",name:t?.name})}const He=class fe{constructor(e,r){R(this,"addr"),R(this,"token"),R(this,"isBinary"),R(this,"connectTimeout"),R(this,"dataTimeout"),R(this,"worker",null),R(this,"connectionTimer",null),R(this,"dataTimer",null),R(this,"_status"),R(this,"dataCallback"),R(this,"rawDataCallback"),R(this,"errorCallback"),R(this,"statusCallback"),this.addr=e.addr||"",this.token=e.token,this.isBinary=e.isBinary??!0,this.connectTimeout=r?.timeout??5e3,this.dataTimeout=r?.dataTimeout??15e3,this._status={main:{status:j.IDLE},connection:{status:ee.DISCONNECTED},auth:{status:oe.UNAUTHENTICATED},data:{status:Y.NONE},worker:{status:se.OFF}}}get status(){return this._status}resolveProtocol(e){let r=e.trim();if(r.startsWith("https://"))return r.replace("https://","wss://");if(r.startsWith("http://"))return r.replace("http://","ws://");if(r.startsWith("wss://")||r.startsWith("ws://"))return r;let i="ws:";return typeof window<"u"&&window.location.protocol==="https:"&&(i="wss:"),`${i}//${r}`}start({dataCallback:e,rawDataCallback:r,errorCallback:i,statusCallback:n}){if(this._status.main.status===j.STARTING||this._status.main.status===j.RUNNING){const o=new Q(x.STREAM_ALREADY_STARTED,"StateStream is already running. Call stop() before starting again.");if(n&&n({...this._status,main:{...this._status.main,lastError:o}}),i)i(o);else throw o;return}this.dataCallback=e,this.rawDataCallback=r,this.errorCallback=i,this.statusCallback=n,this.updateStatusComponent("main",{status:j.STARTING,lastError:void 0});try{this.ensureWorker(),this.sendCommand({type:"START",addr:this.normalizeUrl(this.addr),token:this.token,isBinary:this.isBinary,mode:this.streamMode}),this.clearConnectionTimer(),this.connectionTimer=setTimeout(()=>{const o=new Q(x.CONNECTION_TIMEOUT,`Connection timed out after ${this.connectTimeout}ms`);this.mapErrorToStatus(o),this.errorCallback&&this.errorCallback(o),this.stop()},this.connectTimeout)}catch(o){const s=o instanceof Q?o:new Q(x.UNKNOWN_ERROR,String(o));this.errorCallback&&this.errorCallback(s)}}stop(){this.clearConnectionTimer(),this.clearDataTimer(),!(this._status.main.status===j.IDLE||this._status.main.status===j.STOPPED)&&(this.updateStatusComponent("main",{status:j.STOPPED}),this.updateStatusComponent("connection",{status:ee.DISCONNECTED}),this.sendCommand({type:"STOP"}),this.clearCallbacks())}destroy(){this.stop(),this.worker&&(this.worker.terminate?this.worker.terminate():"close"in this.worker.port&&this.worker.port.close(),this.worker=null)}clearCallbacks(){this.dataCallback=void 0,this.rawDataCallback=void 0,this.errorCallback=void 0,this.statusCallback=void 0}sendToken(e){this.token=e,this.sendCommand({type:"UPDATE_TOKEN",token:e})}sendCommand(e){this.worker&&this.worker.port.postMessage(e)}ensureWorker(){if(this.worker||typeof window>"u")return;const e=btoa(this.addr);try{if(this.updateStatusComponent("worker",{status:se.INITIALIZING,lastError:void 0}),window.SharedWorker){const r=new Mr({name:e});this.worker={port:r.port},r.port.onmessage=i=>{this.handleWorkerMessage(i.data)},r.port.start()}else{const r=new Br;this.worker={port:r,terminate:()=>r.terminate()},r.onmessage=i=>{this.handleWorkerMessage(i.data)}}this.updateStatusComponent("worker",{status:se.READY})}catch(r){const i=new Q(x.WORKER_INIT_FAILED,`Failed to initialize worker: ${String(r)}`);throw this.updateStatusComponent("worker",{status:se.ERROR,lastError:i}),this.updateStatusComponent("main",{status:j.FAILED,lastError:i}),i}}handleWorkerMessage(e){switch(e.type){case"DATA":this.resetDataTimer(),this.dataCallback&&this.dataCallback(this.normalizeState(e.data));break;case"RAW_DATA":this.resetDataTimer(),this.rawDataCallback&&this.rawDataCallback(e.data);break;case"CONNECTED":this.clearConnectionTimer(),this.updateStatusComponent("connection",{status:ee.CONNECTED}),this.streamMode==="local"&&this.updateStatusComponent("main",{status:j.RUNNING});break;case"STATUS_UPDATE":e.connection&&this.updateStatusComponent("connection",{status:e.connection}),e.auth&&(this.updateStatusComponent("auth",{status:e.auth}),e.auth===oe.AUTHENTICATED&&this.updateStatusComponent("main",{status:j.RUNNING}));break;case"ERROR":{this.clearConnectionTimer();const r=new Q(e.code,e.message,e.data);this.mapErrorToStatus(r),this.errorCallback&&this.errorCallback(r);break}case"TOKEN_EXPIRED":this.updateStatusComponent("auth",{status:oe.AUTHENTICATING}),this.handleTokenExpiredInternal();break;case"DISCONNECTED":this.updateStatusComponent("connection",{status:ee.DISCONNECTED});break}}mapErrorToStatus(e){const r=e.code;(r===x.CONNECTION_FAILED||r===x.CONNECTION_LOST||r===x.RECONNECT_FAILED||r===x.CONNECTION_TIMEOUT)&&(this.updateStatusComponent("connection",{status:ee.DISCONNECTED,lastError:e}),this.updateStatusComponent("main",{status:j.FAILED,lastError:e})),(r===x.AUTH_FAILED||r===x.AUTH_REFRESH_FAILED)&&(this.updateStatusComponent("auth",{status:oe.FAILED,lastError:e}),this.updateStatusComponent("main",{status:j.FAILED,lastError:e})),(r===x.DEVICE_ERROR||r===x.DECODE_ERROR)&&this.updateStatusComponent("main",{lastError:e})}updateStatusComponent(e,r){const i=this._status[e];this._status[e]={...i,...r},this.statusCallback&&this.statusCallback({...this._status})}clearConnectionTimer(){this.connectionTimer&&(clearTimeout(this.connectionTimer),this.connectionTimer=null)}resetDataTimer(){this.clearDataTimer(),this._status.data.status!==Y.ACTIVE?this.updateStatusComponent("data",{status:Y.ACTIVE,lastActivity:Date.now()}):this._status.data.lastActivity=Date.now(),this.dataTimer=setTimeout(()=>{this.updateStatusComponent("data",{status:Y.STALE})},this.dataTimeout)}clearDataTimer(){this.dataTimer&&(clearTimeout(this.dataTimer),this.dataTimer=null)}normalizeState(e){let r,i;"bar_id"in e&&"state"in e?(r=e.state,i=e.bar_id):r=e;let n=r.updates;return n&&(n=n.map(o=>{const s=Object.keys(o).find(a=>o[a]!=null);return{...o,state:s}})),{...r,updates:n,bar_id:i}}async handleTokenExpiredInternal(){const e=this.addr;let r=fe.tokenRefreshPromises.get(e);if(!r&&this.onTokenExpired&&(r=(async()=>{try{const i=this.onTokenExpired();return i instanceof Promise?await i:""}finally{fe.tokenRefreshPromises.delete(e)}})(),fe.tokenRefreshPromises.set(e,r)),r){const i=await r;i&&this.sendToken(i)}}};R(He,"tokenRefreshPromises",new Map);let Wr=He;class Gr extends Wr{constructor(e={},r){let i=e.addr;i||(typeof window<"u"?i=window.location.origin:i="10.0.4.20"),super({isBinary:!0,...e,addr:i},r),R(this,"streamMode","local")}normalizeUrl(e){const r=this.resolveProtocol(e),i=new URL(r);return(i.pathname==="/"||!i.pathname)&&(i.pathname="/api/status/ws"),this.token&&i.searchParams.set("x-api-token",this.token),i.toString()}}var C;(t=>{(e=>{e[e.DISCHARGING=0]="DISCHARGING",e[e.CHARGING=1]="CHARGING",e[e.CHARGED=2]="CHARGED"})(t.BatteryStatus||(t.BatteryStatus={})),(e=>{e[e.CONNECTED=0]="CONNECTED",e[e.CONNECTING=1]="CONNECTING",e[e.DISCONNECTING=2]="DISCONNECTING",e[e.RECONNECTING=3]="RECONNECTING"})(t.WifiConnectionStatus||(t.WifiConnectionStatus={})),(e=>{e[e.UNKNOWN=0]="UNKNOWN",e[e.OPEN=1]="OPEN",e[e.WPA=2]="WPA",e[e.WPA2=3]="WPA2",e[e.WEP=4]="WEP",e[e.WPA_WPA2=5]="WPA_WPA2",e[e.WPA3=6]="WPA3",e[e.WPA2_WPA3=7]="WPA2_WPA3"})(t.WifiSecurity||(t.WifiSecurity={})),(e=>{e[e.DHCP=0]="DHCP",e[e.STATIC=1]="STATIC"})(t.IpConfigurationMethod||(t.IpConfigurationMethod={})),(e=>{e[e.IPV4=0]="IPV4",e[e.IPV6=1]="IPV6"})(t.IpProtocol||(t.IpProtocol={})),(e=>{e[e.NEVER_STARTED=0]="NEVER_STARTED",e[e.STARTED=1]="STARTED",e[e.COMPLETED_SUCCESSFULLY=2]="COMPLETED_SUCCESSFULLY",e[e.FAILED=3]="FAILED"})(t.MatterCommissioningStatus||(t.MatterCommissioningStatus={})),(e=>{(r=>{r[r.RESET=0]="RESET",r[r.INITIALIZATION=1]="INITIALIZATION",r[r.READY=2]="READY",r[r.ADVERTISING=3]="ADVERTISING",r[r.CONNECTABLE=4]="CONNECTABLE",r[r.CONNECTED=5]="CONNECTED",r[r.ERROR=6]="ERROR"})(e.ServiceStatus||(e.ServiceStatus={}))})(t.Ble||(t.Ble={}))})(C||(C={}));var he;(t=>{(e=>{e[e.SESSION_START=0]="SESSION_START",e[e.SESSION_STOP=1]="SESSION_STOP",e[e.ACTION_BEGIN=2]="ACTION_BEGIN",e[e.ACTION_DONE=3]="ACTION_DONE",e[e.DETAIL_CHANGE=4]="DETAIL_CHANGE",e[e.ACTION_PROGRESS=5]="ACTION_PROGRESS",e[e.EVENT_NONE=6]="EVENT_NONE"})(t.UpdateEvent||(t.UpdateEvent={})),(e=>{e[e.DOWNLOAD=0]="DOWNLOAD",e[e.SHA_VERIFICATION=1]="SHA_VERIFICATION",e[e.UNPACK=2]="UNPACK",e[e.INSTALLATION_PREPARE=3]="INSTALLATION_PREPARE",e[e.INSTALLATION_APPLY=4]="INSTALLATION_APPLY",e[e.ACTION_NONE=5]="ACTION_NONE"})(t.UpdateAction||(t.UpdateAction={})),(e=>{e[e.OK=0]="OK",e[e.BATTERY_LOW=1]="BATTERY_LOW",e[e.BUSY=2]="BUSY",e[e.DOWNLOAD_FAILURE=3]="DOWNLOAD_FAILURE",e[e.DOWNLOAD_ABORT=4]="DOWNLOAD_ABORT",e[e.SHA_MISMATCH=5]="SHA_MISMATCH",e[e.UNPACK_CREATE_STAGING_DIRECTORY_FAILURE=6]="UNPACK_CREATE_STAGING_DIRECTORY_FAILURE",e[e.UNPACK_ARCHIVE_OPEN_FAILURE=7]="UNPACK_ARCHIVE_OPEN_FAILURE",e[e.UNPACK_ARCHIVE_UNPACK_FAILURE=8]="UNPACK_ARCHIVE_UNPACK_FAILURE",e[e.INSTALLATION_PREPARE_MANIFEST_NOT_FOUND=9]="INSTALLATION_PREPARE_MANIFEST_NOT_FOUND",e[e.INSTALLATION_PREPARE_MANIFEST_INVALID=10]="INSTALLATION_PREPARE_MANIFEST_INVALID",e[e.INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE=11]="INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE",e[e.INSTALLATION_PREPARE_POINTER_SETUP_FAILURE=12]="INSTALLATION_PREPARE_POINTER_SETUP_FAILURE",e[e.UNKNOWN_FAILURE=13]="UNKNOWN_FAILURE"})(t.UpdateStatus||(t.UpdateStatus={})),(e=>{e[e.NOT_AVAILABLE=0]="NOT_AVAILABLE",e[e.FAILURE=1]="FAILURE",e[e.IDLE=2]="IDLE"})(t.CheckError||(t.CheckError={}))})(he||(he={}));var Ne;(t=>{(e=>{e[e.PLAIN=0]="PLAIN",e[e.RUN_LENGTH=1]="RUN_LENGTH",e[e.DEFLATE=2]="DEFLATE",e[e.DEFLATE_RUN_LENGTH=3]="DEFLATE_RUN_LENGTH"})(t.Encoding||(t.Encoding={})),(e=>{e[e.RGB888=0]="RGB888",e[e.L8=1]="L8",e[e.L4=2]="L4"})(t.PixelFormat||(t.PixelFormat={})),(e=>{e[e.FRONT=0]="FRONT",e[e.BACK=1]="BACK"})(t.Screen||(t.Screen={}))})(Ne||(Ne={}));var Ce;(t=>{(e=>{e[e.PLAIN=0]="PLAIN",e[e.GZIP=1]="GZIP"})(t.Compression||(t.Compression={}))})(Ce||(Ce={}));var ke;(t=>{(e=>{e[e.OK=0]="OK",e[e.BACK=1]="BACK",e[e.START=2]="START"})(t.Button||(t.Button={})),(e=>{e[e.PRESS=0]="PRESS",e[e.RELEASE=1]="RELEASE"})(t.ButtonAction||(t.ButtonAction={})),(e=>{e[e.BUSY=0]="BUSY",e[e.CUSTOM=1]="CUSTOM",e[e.OFF=2]="OFF",e[e.APPS=3]="APPS",e[e.SETTINGS=4]="SETTINGS"})(t.SwitchPosition||(t.SwitchPosition={}))})(ke||(ke={}));var Ie;(t=>{(e=>{e[e.RESOURCE_LIMIT=0]="RESOURCE_LIMIT"})(t.Cause||(t.Cause={})),(e=>{e[e.FATAL=0]="FATAL",e[e.ERROR=1]="ERROR",e[e.WARNING=2]="WARNING"})(t.Severity||(t.Severity={}))})(Ie||(Ie={}));const Je=class de{constructor(){if(R(this,"gl",null),R(this,"program",null),R(this,"texture",null),R(this,"vs",`#version 300 es
    in vec2 position;
    out vec2 v_uv;
    void main() {
      v_uv = position * 0.5 + 0.5;
      v_uv.y = 1.0 - v_uv.y;
      gl_Position = vec4(position, 0, 1);
    }
  `),R(this,"fs",`#version 300 es
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
  `),de.instance)return de.instance;de.instance=this}init(){if(this.gl||typeof window>"u")return;const e=document.createElement("canvas").getContext("webgl2",{antialias:!0,alpha:!0,preserveDrawingBuffer:!0});if(!e)throw new Error("WebGL 2.0 not supported");this.gl=e,this.program=this.createProgram(this.vs,this.fs);const r=e.createBuffer();e.bindBuffer(e.ARRAY_BUFFER,r),e.bufferData(e.ARRAY_BUFFER,new Float32Array([-1,-1,1,-1,-1,1,-1,1,1,-1,1,1]),e.STATIC_DRAW);const i=e.getAttribLocation(this.program,"position");e.enableVertexAttribArray(i),e.vertexAttribPointer(i,2,e.FLOAT,!1,0,0),this.texture=e.createTexture(),e.bindTexture(e.TEXTURE_2D,this.texture),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_MIN_FILTER,e.NEAREST),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_MAG_FILTER,e.NEAREST),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_WRAP_S,e.CLAMP_TO_EDGE),e.texParameteri(e.TEXTURE_2D,e.TEXTURE_WRAP_T,e.CLAMP_TO_EDGE)}render(e,r,i,n){const{pixelSize:o=.85,radius:s=.5,darkThreshold:a=.04}=n,u=this.gl;u.viewport(0,0,u.canvas.width,u.canvas.height),u.useProgram(this.program),u.bindTexture(u.TEXTURE_2D,this.texture),u.texImage2D(u.TEXTURE_2D,0,u.RGBA8,r,i,0,u.RGBA,u.UNSIGNED_BYTE,e),u.uniform2f(u.getUniformLocation(this.program,"u_dataRes"),r,i),u.uniform2f(u.getUniformLocation(this.program,"u_canvasRes"),u.canvas.width,u.canvas.height),u.uniform1f(u.getUniformLocation(this.program,"u_pixelSize"),o),u.uniform1f(u.getUniformLocation(this.program,"u_radius"),s),u.uniform1f(u.getUniformLocation(this.program,"u_darkThreshold"),a),u.clearColor(0,0,0,0),u.clear(u.COLOR_BUFFER_BIT),u.enable(u.BLEND),u.blendFunc(u.SRC_ALPHA,u.ONE_MINUS_SRC_ALPHA),u.drawArrays(u.TRIANGLES,0,6)}renderFrame(e,r,i,n,o={}){if(this.init(),typeof window>"u"||!this.gl)return;(this.gl.canvas.width!==e.width||this.gl.canvas.height!==e.height)&&(this.gl.canvas.width=e.width,this.gl.canvas.height=e.height),this.render(r,i,n,o);const s=e.getContext("2d");s&&(s.clearRect(0,0,e.width,e.height),s.drawImage(this.gl.canvas,0,0))}createProgram(e,r){const i=this.gl,n=(s,a)=>{const u=i.createShader(s);if(i.shaderSource(u,a),i.compileShader(u),!i.getShaderParameter(u,i.COMPILE_STATUS))throw new Error(i.getShaderInfoLog(u)||"Shader Error");return u},o=i.createProgram();return i.attachShader(o,n(i.VERTEX_SHADER,e)),i.attachShader(o,n(i.FRAGMENT_SHADER,r)),i.linkProgram(o),o}};R(Je,"instance",null);let $r=Je;const Ti=new $r,te=V("apiStore",()=>{const t=me().public.barUrl||window.location.origin,e=O(null);async function r(i,n){const o={...n?.headers||{}};return e.value&&(o["X-API-Token"]=e.value),$fetch(i,{baseURL:t,...n,headers:o})}return{apiKey:e,apiRequest:r}},{persist:{key:"apiStore",storage:et.sessionStorage()}});async function T(t,e,r,i){const n=J();if(t?.status===403){await tt("/login");return}console.error(e,t),r&&await n.checkConnection(),n.isConnected&&L.add({id:"device-status-error",title:e,description:zr(t),icon:"i-bi-alert",color:"error",duration:typeof i=="number"?i:1e4})}const Vr="Unknown error. Check your connection and try again.";function zr(t){if(t?.data?.error)return t.data.error;if(String(t).length){const e=String(t);if(e.includes("Error:")){const r=e.indexOf("Error:");return e.slice(r+6).trim()}return e}return Vr}const be=globalThis.setInterval;var z=(t=>(t[t.IDLE=0]="IDLE",t[t.LOADING=1]="LOADING",t[t.UPDATING=2]="UPDATING",t[t.ERROR=3]="ERROR",t[t.SUCCESS=4]="SUCCESS",t))(z||{});const we=V("firmware",()=>{const t=J(),e=1800*1e3,r=O({status:null,availableVersion:null,isAllowed:null,isChecking:!1,isManualCheck:!1,backgroundCheckInterval:null,modals:{changelog:!1,batteryLow:!1,updating:!1,success:!1},changelog:null,isChangelogLoading:!1,stage:0,progress:0,progressPollingInterval:null,error:{stage:0,message:null}}),i=O({is_enabled:!1,interval_start:"02:00",interval_end:"05:00"});async function n(){return te().apiRequest("/api/update/autoupdate").then(p=>(i.value={is_enabled:p.is_enabled,interval_start:p.interval_start,interval_end:p.interval_end},i.value)).catch(async p=>{await T(p,"Couldn't fetch auto-update self-check settings")})}async function o(p){return te().apiRequest("/api/update/autoupdate",{method:"POST",body:p}).then(()=>{i.value=p}).catch(async w=>{await T(w,"Couldn't update auto-update self-check settings")})}function s(){r.value.status=null,r.value.availableVersion=null,r.value.isAllowed=null,r.value.changelog=null,r.value.progress=0,r.value.progressPollingInterval&&(clearInterval(r.value.progressPollingInterval),r.value.progressPollingInterval=null),r.value.error.stage=0,r.value.error.message=null}async function a(p=0){return t.busyBar.UpdateStatusGet({timeout:1e4}).then(async w=>{if(!w.check?.status||!w.check.event)throw new Error("Invalid update status response: missing check info");if(!w.install)throw new Error("Invalid update status response: missing install info");if(w.check.event==="stop"&&w.check.status==="failure"){console.warn("Auto-update check failed",w),r.value.isChecking=!1,r.value.isManualCheck&&(r.value.isManualCheck=!1,L.add({title:"Update check failed",description:"Check your internet connection and try again.",icon:"i-bi-alert",color:"error",duration:1e4}));return}if(w.check.event!=="stop"&&w.check.status==="none"){if(w.check.event==="none")return console.debug("Empty auto update status, requesting update check"),u();if(console.debug("Auto-update check still in progress, fetching status again"),await new Promise(P=>{setTimeout(P,3e3)}),p>=10)throw new Error("Auto-update check is taking too long, please try again later");return a(p?p+1:1)}if(r.value.isChecking=!1,r.value.status=w.check.status||null,r.value.availableVersion=w.check.available_version||null,r.value.isAllowed=!!w.install.is_allowed,r.value.isManualCheck&&w.check.status==="not_available"&&(r.value.isManualCheck=!1,L.add({title:"Your firmware version is up to date",icon:"i-bi-checkmark-circle-fill",color:"success",duration:1e4})),console.debug("Auto-update check completed",w),r.value.availableVersion)return l(r.value.availableVersion)}).catch(async w=>{r.value.isChecking=!1,await T(w,"Couldn't check for updates")})}async function u(){if(r.value.isChecking){console.debug("Already checking for updates, ignoring request");return}return r.value.isChecking=!0,t.busyBar.UpdateCheck({timeout:1e4}).then(async()=>(console.debug("Auto-update check requested"),await new Promise(p=>setTimeout(p,1e3)),a())).catch(async p=>{if(p.status===409){console.debug("Auto-update check already in progress");return}r.value.isChecking=!1,await T(p,"Update check request failed")})}function c(){r.value.backgroundCheckInterval=be(()=>{console.debug(`Performing background auto-update check (${new Date().toISOString()})`),u()},e)}function d(){r.value.backgroundCheckInterval&&(clearInterval(r.value.backgroundCheckInterval),r.value.backgroundCheckInterval=null)}async function l(p){r.value.isChangelogLoading=!0,await t.busyBar.UpdateChangelogGet({version:p}).then(w=>{r.value.changelog=w.changelog||null}).catch(async w=>(await T(w,"Couldn't fetch update changelog"),null)).finally(()=>{r.value.isChangelogLoading=!1})}async function f(){if(!r.value.availableVersion){console.error("No available version to install");return}return console.debug("Requesting auto-update installation"),t.busyBar.UpdateInstall({version:r.value.availableVersion,timeout:1e4})}async function h(){await t.busyBar.UpdateAbort().then(()=>{console.debug("Auto-update download abort requested"),r.value.modals.updating=!1,r.value.stage=0,r.value.progress=0}).catch(async p=>{await T(p,"Couldn't abort update download")})}async function g(){console.debug("Starting auto-update process"),r.value.progress=0,r.value.error.stage=0,r.value.error.message=null,r.value.stage=1,await f().catch(async p=>{await T(p,"Update failed")}),r.value.progressPollingInterval=be(async()=>{await t.busyBar.UpdateStatusGet({timeout:1e4}).then(p=>{if(!p.install)throw new Error("Invalid update status response: missing install info");if(p.install.event==="session_stop"){if(r.value.progressPollingInterval&&(clearInterval(r.value.progressPollingInterval),r.value.progressPollingInterval=null),p.install.status==="ok")return;if(p.install.status==="busy"){console.warn("Received session_stop event with status busy. Is this a firmware bug?");return}else if(p.install.status==="download_abort"){console.warn("Update download was aborted"),r.value.modals.updating=!1,r.value.stage=0,r.value.progress=0,L.add({title:"Update aborted",description:"The update download has been aborted.",duration:1e4});return}r.value.error.stage=r.value.stage,r.value.error.message=`Update failed with status: ${p.install.status}`,r.value.stage=3;return}if(p.install.status!=="ok"&&p.install.status!=="busy"){console.error("Update failed with status:",p),r.value.error.stage=r.value.stage,r.value.error.message=`Update failed: ${p.install.status}`,r.value.stage=3,clearInterval(r.value.progressPollingInterval);return}if(p.install.action==="download"){let w=Number(p.install.download?.total_bytes);isNaN(w)&&(console.warn("Received invalid total_bytes value in update status, defaulting to 0",p.install.download?.total_bytes),w=0);let P=Number(p.install.download?.received_bytes);isNaN(P)&&(console.warn("Received invalid received_bytes value in update status, defaulting to 0",p.install.download?.received_bytes),P=0),r.value.stage=1,w>0&&(r.value.progress=Math.round(P/w*100))}else p.install.action!=="none"&&(r.value.stage=2,r.value.progress=0,clearInterval(r.value.progressPollingInterval))}).catch(async p=>{await T(p,"Couldn't fetch update status"),r.value.stage=3,clearInterval(r.value.progressPollingInterval)})},1e3),r.value.modals.changelog=!1,r.value.modals.updating=!0}const m=O({firmwareBundleName:"firmware",firmwareFile:null,showFileUploadModal:!1,stage:0,progress:0,error:null});async function E(){const p=new XMLHttpRequest;p.open("POST",`${me().public.barUrl||window.location.origin}/api/update`),p.setRequestHeader("Content-Type","application/octet-stream"),te().apiKey&&p.setRequestHeader("X-API-Token",te().apiKey),p.upload.onprogress=w=>{w.lengthComputable&&(m.value.progress=Math.round(w.loaded/w.total*100),r.value.progress=m.value.progress,m.value.progress===100&&console.debug("Firmware file upload completed, waiting for device to unpack"))},p.onload=()=>{p.status>=200&&p.status<400?(console.debug("Upload and unpacking complete, waiting for device to reboot"),m.value.stage=2,L.add({title:"Update initiated",description:"The device will reboot to apply the update. Pay attention to the front screen.",icon:"i-bi-checkmark-circle-fill",color:"success",duration:1e4})):(console.error("Upload failed:",p.status,p.responseText),m.value.stage=3,L.add({title:"Update failed",description:`Error ${p.status}: ${p.responseText}`,icon:"i-bi-alert",color:"error",duration:1e4}),m.value.error=`Error ${p.status}: ${p.responseText}`)},p.onerror=()=>{console.error("Upload error"),m.value.stage=3,L.add({title:"Update failed",description:"An error occurred during the upload.",icon:"i-bi-alert",color:"error",duration:1e4}),m.value.error="An error occurred during the upload."},m.value.stage=1,m.value.progress=0,p.send(m.value.firmwareFile),await new Promise(w=>{p.onloadend=()=>{w()}}),m.value.firmwareFile=null,m.value.stage!==3&&(m.value.progress=0)}async function _(){try{m.value.showFileUploadModal=!1,r.value.modals.updating=!0,await E(),m.value.stage!==3&&(m.value.stage=2)}catch(p){console.error("Firmware update failed:",p),m.value.stage=3,m.value.error=p instanceof Error?p.message:"Unknown error"}}return{autoUpdateSelfCheck:i,fetchAutoUpdateSelfCheck:n,setAutoUpdateSelfCheck:o,autoUpdate:r,resetAutoUpdateState:s,fetchAutoUpdateStatus:a,requestAutoUpdateCheck:u,setAutoUpdateBackgroundCheckInterval:c,clearAutoUpdateBackgroundCheckInterval:d,startAutoUpdate:g,abortAutoUpdateDownload:h,fileUpdate:m,uploadFirmware:E,startFirmwareUpdateFromFile:_}}),J=V("device",()=>{const t=te().apiRequest,e=Ze(),r=we(),i=Me(new Ve({addr:me().public.barUrl||window.location.origin})),n=O(!0),o=O(!1);async function s(){if(o.value)return"aborted";o.value=!0;const A=n.value;try{await t("/api/name",{timeout:3e3}),n.value||(window.dispatchEvent(new Event("device-reconnected")),r.autoUpdate.stage===z.UPDATING&&(r.autoUpdate.stage=z.SUCCESS)),n.value=!0,console.debug("Device is connected"),L.remove("device-disconnected")}catch(b){if(!a.value){console.debug("conncheck request aborted, ignoring because refresh interval is cleared");const y=b;if(y?.name==="AbortError"||y?.message?.toLowerCase().includes("abort")||y?.code==="ECONNABORTED")return o.value=!1,"aborted"}A&&window.dispatchEvent(new Event("device-disconnected")),n.value=!1,console.debug("Device is disconnected"),r.autoUpdate.stage!==z.UPDATING&&!(r.autoUpdate.stage===z.SUCCESS&&e.wifi?.state!=="connected")&&(r.fileUpdate.stage===z.IDLE||r.fileUpdate.stage===z.ERROR)&&L.add({id:"device-disconnected",title:"Device disconnected",description:"Device lost. Please check the connection.",icon:"i-bi-alert",color:"warning",duration:0,close:!0,closeIcon:"i-bi-cross"})}return o.value=!1,n.value}const a=O();async function u(){const A=we();if(A.autoUpdate.stage===z.LOADING||A.fileUpdate.stage===z.LOADING){console.debug("Skipping connection check during auto update");return}if(await s(),!n.value)return;L.remove("device-disconnected"),await E();const b=e.wifi?.state,y=await e.fetchWifiState();b!==y?.state&&(y?.state==="connected"?window.dispatchEvent(new Event("wifi-reconnected")):window.dispatchEvent(new Event("wifi-disconnected"))),await F()}function c(){a.value=be(u,5e3)}function d(){a.value&&(clearInterval(a.value),a.value=void 0)}const l=O("wifi");async function f(){await i.value.SystemTransportGet().then(A=>{l.value=A.type,console.debug("Detected connection type:",l.value)}).catch(async A=>(await T(A,"Couldn't get connection type",!0),l.value))}const h=O(void 0);async function g(){return await i.value.SystemVersionGet().then(b=>(h.value=b,b)).catch(async b=>(await T(b,"Couldn't get HTTP API version",!0),h.value))}const m=O(void 0);async function E(){return await i.value.SystemStatusGet().then(b=>(m.value=b,b)).catch(async b=>(await T(b,"Couldn't get device status",!0),m.value))}const _="BUSY Bar",p=O(void 0);async function w(A=!1){return await i.value.SettingsNameGet().then(y=>(p.value=y.name,y.name)).catch(async y=>{if(A)throw y;return await T(y,"Couldn't get device name"),_})}async function P(A){return await i.value.SettingsNameSet({name:A}).then(()=>(p.value=A,L.add({title:"Changes saved",icon:"i-bi-checkmark-circle-fill",color:"success"}),!0)).catch(async b=>(await T(b,"Couldn't set device name"),!1))}const B=O(void 0);async function F(){return await i.value.SettingsAccessGet().then(b=>(B.value=b,b)).catch(async b=>(await T(b,"Couldn't get HTTP API access state",!0),B.value))}async function G(A,b){const y={mode:A};if(A==="key"){if(!b)throw new Error("Password not provided");y.key=b}return await i.value.SettingsAccessSet(y).then(async()=>(B.value=await F(),L.add({title:A==="key"?"Password set":"Changes saved",icon:"i-bi-checkmark-circle-fill",color:"success"}),!0)).catch(async I=>(await T(I,"Couldn't set HTTP API access state"),!1))}return{busyBar:i,isConnected:n,checkConnection:s,connectionType:l,detectConnectionType:f,refreshInterval:a,setRefreshInterval:c,clearRefreshInterval:d,apiVersion:h,fetchApiVersion:g,deviceStatus:m,fetchDeviceStatus:E,deviceName:p,fetchDeviceName:w,setDeviceName:P,httpAPIAccess:B,fetchHttpAPIAccess:F,setHttpAPIAccess:G}}),Hr=V("audio",()=>{const t=J(),e=O(void 0);async function r(){return await t.busyBar.AudioVolumeGet().then(o=>(e.value=o,o)).catch(async o=>(await T(o,"Couldn't get audio volume",!0),e.value))}async function i(n){return await t.busyBar.AudioVolumeSet({volume:n}).then(()=>(e.value?e.value.volume=n:e.value={volume:n},!0)).catch(async o=>(await T(o,"Couldn't set audio volume"),!1))}return{audio:e,fetchAudioVolume:r,setAudioVolume:i}}),Jr=V("brightness",()=>{const t=J(),e=O(void 0);async function r(){return await t.busyBar.DisplayBrightnessGet().then(o=>{const a={value:o.value==="auto"?"auto":Number(o.value)};return e.value=a,a}).catch(async o=>(await T(o,"Couldn't get display brightness",!0),e.value))}async function i(n){return await t.busyBar.DisplayBrightnessSet(n).then(()=>(e.value=n,!0)).catch(async o=>(await T(o,"Couldn't set display brightness"),!1))}return{displayBrightness:e,fetchDisplayBrightness:r,setDisplayBrightness:i}});/*!
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
*/const ie={newline:10,reset:27};function Kr(t){if(!Number.isSafeInteger(t))throw new Error(`integer expected: ${t}`)}function Xr(t){if(!Number.isSafeInteger(t)||t<1||t>40)throw new Error(`Invalid version=${t}. Expected number [1..40]`)}function K(t,e){return t.toString(2).padStart(e,"0")}function qe(t,e){const r=t%e;return r>=0?r:e+r}function $(t,e){return new Array(t).fill(e)}function ge(t){return t=t-(t>>>1&1431655765),t=(t&858993459)+(t>>>2&858993459),(t+(t>>>4)&252645135)*16843009>>>24}function Pe(t){let e=0,r=0;for(const o of t)e=Math.max(e,o.length),r+=o.length;const i=new Uint8Array(r);let n=0;for(let o=0;o<e;o++)for(const s of t)o<s.length&&(i[n++]=s[o]);return i}function Yr(){let t,e=1/0;return{add(r,i){r>=e||(t=i,e=r)},get:()=>t,score:()=>e}}function De(t){return{has:e=>t.includes(e),decode:e=>{if(!Array.isArray(e)||e.length&&typeof e[0]!="string")throw new Error("alphabet.decode input should be array of strings");return e.map(r=>{if(typeof r!="string")throw new Error(`alphabet.decode: not string element=${r}`);const i=t.indexOf(r);if(i===-1)throw new Error(`Unknown letter: "${r}". Allowed: ${t}`);return i})},encode:e=>{if(!Array.isArray(e)||e.length&&typeof e[0]!="number")throw new Error("alphabet.encode input should be an array of numbers");return e.map(r=>{if(Kr(r),r<0||r>=t.length)throw new Error(`Digit index outside alphabet: ${r} (alphabet: ${t.length})`);return t[r]})}}}function xe(t){if(t.length!==32)throw new Error("expects 32 element matrix");const e=[1431655765,858993459,252645135,16711935,65535];for(let r=0;r<5;r++){const i=e[r]>>>0,n=1<<r,o=n<<1;for(let s=0;s<32;s+=o)for(let a=0;a<n;a++){const u=s+a,c=u+n,d=t[u]>>>0,l=t[c]>>>0,f=(d>>>n^l)&i;t[u]=(d^f<<n)>>>0,t[c]=(l^f)>>>0}}}const ne=t=>1<<(t&31)>>>0,X=(t,e)=>e===0?0:e===32?4294967295:(1<<e)-1<<t>>>0;class D{static size(e,r){if(typeof e=="number"&&(e={height:e,width:e}),!Number.isSafeInteger(e.height)&&e.height!==1/0)throw new Error(`Bitmap: invalid height=${e.height} (${typeof e.height})`);if(!Number.isSafeInteger(e.width)&&e.width!==1/0)throw new Error(`Bitmap: invalid width=${e.width} (${typeof e.width})`);return r!==void 0&&(e={width:Math.min(e.width,r.width),height:Math.min(e.height,r.height)}),e}static fromString(e){e=e.replace(/^\n+/g,"").replace(/\n+$/g,"");const r=e.split(String.fromCharCode(ie.newline)),i=r.length;let n;const o=[];for(const s of r){const a=s.split("").map(u=>{if(u==="X")return!0;if(u===" ")return!1;if(u!=="?")throw new Error(`Bitmap.fromString: unknown symbol=${u}`)});if(n!==void 0&&a.length!==n)throw new Error(`Bitmap.fromString different row sizes: width=${n} cur=${a.length}`);n=a.length,o.push(a)}return n===void 0&&(n=0),new D({height:i,width:n},o)}defined;value;tailMask;words;fullWords;height;width;constructor(e,r){const{height:i,width:n}=D.size(e);if(this.height=i,this.width=n,this.tailMask=X(0,n&31||32),this.words=Math.ceil(n/32)|0,this.fullWords=Math.floor(n/32)|0,this.value=new Uint32Array(this.words*i),this.defined=new Uint32Array(this.value.length),r){if(r.length!==i)throw new Error(`Bitmap: data height mismatch: exp=${i} got=${r.length}`);for(let o=0;o<i;o++){const s=r[o];if(!s||s.length!==n)throw new Error(`Bitmap: data width mismatch at y=${o}: exp=${n} got=${s?.length}`);for(let a=0;a<n;a++)this.set(a,o,s[a])}}}point(e){return this.get(e.x,e.y)}isInside(e){return 0<=e.x&&e.x<this.width&&0<=e.y&&e.y<this.height}size(e){if(!e)return{height:this.height,width:this.width};const{x:r,y:i}=this.xy(e);return{height:this.height-i,width:this.width-r}}xy(e){if(typeof e=="number"&&(e={x:e,y:e}),!Number.isSafeInteger(e.x))throw new Error(`Bitmap: invalid x=${e.x}`);if(!Number.isSafeInteger(e.y))throw new Error(`Bitmap: invalid y=${e.y}`);return e.x=qe(e.x,this.width),e.y=qe(e.y,this.height),e}wordIndex(e,r){return r*this.words+(e>>>5)}bitIndex(e,r){return{word:this.wordIndex(e,r),bit:e&31}}isDefined(e,r){const i=this.wordIndex(e,r),n=ne(e);return(this.defined[i]&n)!==0}get(e,r){const i=this.wordIndex(e,r),n=ne(e);return(this.value[i]&n)!==0}maskWord(e,r,i){const{defined:n,value:o}=this;n[e]|=r,o[e]=o[e]&~r|-i&r}set(e,r,i){i!==void 0&&this.maskWord(this.wordIndex(e,r),ne(e),i)}fillRectConst(e,r,i,n,o){if(i<=0||n<=0||o===void 0)return;const{value:s,defined:a,words:u}=this,c=e>>>5,d=e+i-1>>>5,l=e&31,f=e+i-1&31;for(let h=0;h<n;h++){const g=(r+h)*u;if(c===d){const m=X(l,f-l+1);this.maskWord(g+c,m,o);continue}this.maskWord(g+c,X(l,32-l),o);for(let m=c+1;m<d;m++)a[g+m]=4294967295,s[g+m]=o?4294967295:0;this.maskWord(g+d,X(0,f+1),o)}}rectWords(e,r,i,n,o){for(let s=0;s<n;s++){const a=r+s;for(let u=0;u<i;){const c=e+u,{bit:d,word:l}=this.bitIndex(c,a),f=Math.min(32-d,i-u);o(l,c,u,s,f),u+=f}}}rect(e,r,i){const{x:n,y:o}=this.xy(e),{height:s,width:a}=D.size(r,this.size({x:n,y:o}));if(typeof i!="function")return this.fillRectConst(n,o,a,s,i),this;const{defined:u,value:c}=this;return this.rectWords(n,o,a,s,(d,l,f,h,g)=>{let m=0,E=c[d];for(let _=0;_<g;_++){const p=ne(l+_),w=i({x:f+_,y:h},(E&p)!==0);w!==void 0&&(m|=p,E=E&~p|-w&p)}u[d]|=m,c[d]=E}),this}rectRead(e,r,i){const{x:n,y:o}=this.xy(e),{height:s,width:a}=D.size(r,this.size({x:n,y:o})),{value:u}=this;return this.rectWords(n,o,a,s,(c,d,l,f,h)=>{const g=u[c];for(let m=0;m<h;m++){const E=ne(d+m);i({x:l+m,y:f},(g&E)!==0)}}),this}hLine(e,r,i){return this.rect(e,{width:r,height:1},i)}vLine(e,r,i){return this.rect(e,{width:1,height:r},i)}border(e=2,r){const i=this.height+2*e,n=this.width+2*e,o=new D({height:i,width:n});return o.rect(0,1/0,r),o.embed({x:e,y:e},this),o}embed(e,r){const{x:i,y:n}=this.xy(e),{height:o,width:s}=D.size(r.size(),this.size({x:i,y:n}));if(s<=0||o<=0)return this;const{value:a,defined:u}=this,{words:c,value:d}=r;for(let l=0;l<o;l++){const f=l*c;for(let h=0;h<s;){const g=i+h,{word:m,bit:E}=this.bitIndex(g,n+l),{word:_,bit:p}=r.bitIndex(h,l),w=Math.min(32-E,s-h),P=d[_],B=p&&_+1<f+c?d[_+1]:0,F=p?(P>>>p|B<<32-p)>>>0:P,G=X(E,w),A=(F&X(0,w))<<E>>>0;u[m]|=G,a[m]=a[m]&~G|A,h+=w}}return this}rectSlice(e,r=this.size()){const{x:i,y:n}=this.xy(e),{height:o,width:s}=D.size(r,this.size({x:i,y:n})),a=new D({height:o,width:s});return this.rectRead({x:i,y:n},{height:o,width:s},(u,c)=>{this.isDefined(i+u.x,n+u.y)&&a.set(u.x,u.y,c)}),a}transpose(){const{height:e,width:r,value:i,defined:n,words:o}=this,s=new D({height:r,width:e}),{words:a,value:u,defined:c,tailMask:d}=s,l=new Uint32Array(32),f=new Uint32Array(32);for(let h=0;h<e;h+=32)for(let g=0;g<o;g++){const m=Math.min(32,e-h);for(let E=0;E<m;E++){const _=this.wordIndex(32*g,h+E);l[E]=i[_],f[E]=n[_]}l.fill(0,m),f.fill(0,m),xe(l),xe(f);for(let E=0;E<32;E++){const _=g*32+E;if(_>=r)break;const p=s.wordIndex(h,_),w=h>>>5===a-1?d:4294967295;u[p]=l[E]&w,c[p]=f[E]&w}}return s}negate(){const e=this.defined.length;for(let r=0;r<e;r++)this.value[r]=~this.value[r],this.defined[r]=4294967295;return this}scale(e){if(!Number.isSafeInteger(e)||e>1024)throw new Error(`invalid scale factor: ${e}`);const{height:r,width:i}=this;return new D({height:e*r,width:e*i}).rect({x:0,y:0},1/0,({x:o,y:s})=>this.get(o/e|0,s/e|0))}clone(){const e=new D(this.size());return e.defined.set(this.defined),e.value.set(this.value),e}assertDrawn(){const{height:e,width:r,defined:i,tailMask:n,fullWords:o,words:s}=this;if(!(!e||!r))for(let a=0;a<e;a++){const u=a*s;for(let c=0;c<o;c++)if(i[u+c]!==4294967295)throw new Error("Invalid color type=undefined");if(s!==o&&(i[u+o]&n)!==n)throw new Error("Invalid color type=undefined")}}countPatternInRow(e,r,...i){if(r<=0||r>=32)throw new Error("wrong patternLen");const n=(1<<r)-1,{width:o,value:s,words:a}=this;let u=0;const c=this.wordIndex(0,e);for(let d=0,l=0;d<a;d++){const f=s[c+d],h=d===a-1&&o&31||32;for(let g=0;g<h;g++)if(l=(l<<1|f>>>g&1)&n,!(d*32+g+1<r)){for(const m of i)if(l===m){u++;break}}}return u}getRuns(e,r){const{width:i,value:n,words:o}=this;if(i===0)return;let s=0,a;const u=this.wordIndex(0,e);for(let c=0;c<o;c++){const d=n[u+c],l=c===o-1&&i&31||32;for(let f=0;f<l;f++){const h=(d&1<<f)!==0;if(h===a){s++;continue}a!==void 0&&r(s,a),a=h,s=1}}a!==void 0&&r(s,a)}popcnt(){const{height:e,width:r,words:i,fullWords:n,tailMask:o}=this;if(!e||!r)return 0;let s=0;for(let a=0;a<e;a++){const u=a*i;for(let c=0;c<n;c++)s+=ge(this.value[u+c]);i!==n&&(s+=ge(this.value[u+n]&o))}return s}countBoxes2x2(e){const{width:r,words:i}=this;if(r<2||(e|0)<0||e+1>=this.height)return 0;const n=this.wordIndex(0,e)|0,o=this.wordIndex(0,e+1)|0,a=(r&31)===0?2147483647:X(0,r-1&31);let u=0;for(let c=0;c<i;c++){const d=this.value[n+c],l=this.value[o+c],f=~(d^l)>>>0,h=c+1<i?this.value[n+c+1]>>>0:0,g=~(d^(d>>>1|(h&1)<<31)>>>0)>>>0,m=c+1<i?this.value[o+c+1]>>>0:0,E=~(l^(l>>>1|(m&1)<<31)>>>0)>>>0;let _=(f&g&E)>>>0;c===i-1&&(_&=a),u+=ge(_)}return u}toString(){const e=String.fromCharCode(ie.newline);let r="";for(let i=0;i<this.height;i++){let n="";for(let o=0;o<this.width;o++){const s=this.get(o,i);n+=this.isDefined(o,i)?s?"X":" ":"?"}r+=n+(i+1===this.height?"":e)}return r}toRaw(){const e=Array.from({length:this.height},()=>new Array(this.width));for(let r=0;r<this.height;r++){const i=e[r];for(let n=0;n<this.width;n++)i[n]=this.get(n,r)}return e}toASCII(){const{height:e,width:r}=this;let i="";for(let n=0;n<e;n+=2){for(let o=0;o<r;o++){const s=this.get(o,n),a=n+1>=e?!0:this.get(o,n+1);!s&&!a?i+="█":!s&&a?i+="▀":s&&!a?i+="▄":s&&a&&(i+=" ")}i+=String.fromCharCode(ie.newline)}return i}toTerm(){const e=String.fromCharCode(ie.reset),r=e+"[0m",i=e+"[1;47m  "+r,n=e+"[40m  "+r,o=String.fromCharCode(ie.newline);let s="";for(let a=0;a<this.height;a++){for(let u=0;u<this.width;u++){const c=this.get(u,a);s+=c?n:i}s+=o}return s}toSVG(e=!0){let r=`<svg viewBox="0 0 ${this.width} ${this.height}" xmlns="http://www.w3.org/2000/svg">`,i="",n;return this.rectRead(0,1/0,(o,s)=>{if(!s)return;const{x:a,y:u}=o;if(!e){r+=`<rect x="${a}" y="${u}" width="1" height="1" />`;return}let c=`M${a} ${u}`;if(n){const l=`m${a-n.x} ${u-n.y}`;l.length<=c.length&&(c=l)}const d=a<10?`H${a}`:"h-1";i+=`${c}h1v1${d}Z`,n=o}),e&&(r+=`<path d="${i}"/>`),r+="</svg>",r}toGIF(){const e=a=>[a&255,a>>>8&255],r=[...e(this.width),...e(this.height)],i=[];this.rectRead(0,1/0,(a,u)=>i.push(+(u===!0)));const n=126,o=[71,73,70,56,55,97,...r,246,0,0,255,255,255,...$(381,0),44,0,0,0,0,...r,0,7],s=Math.floor(i.length/n);for(let a=0;a<s;a++)o.push(n+1,128,...i.slice(n*a,n*(a+1)).map(u=>+u));return o.push(i.length%n+1,128,...i.slice(s*n).map(a=>+a)),o.push(1,129,0,59),new Uint8Array(o)}toImage(e=!1){const{height:r,width:i}=this.size(),n=new Uint8Array(r*i*(e?3:4));let o=0;for(let s=0;s<r;s++)for(let a=0;a<i;a++){const u=this.get(a,s)?0:255;n[o++]=u,n[o++]=u,n[o++]=u,e||(n[o++]=255)}return{height:r,width:i,data:n}}}const je=["low","medium","quartile","high"],Ue=["numeric","alphanumeric","byte","kanji","eci"],Zr=[26,44,70,100,134,172,196,242,292,346,404,466,532,581,655,733,815,901,991,1085,1156,1258,1364,1474,1588,1706,1828,1921,2051,2185,2323,2465,2611,2761,2876,3034,3196,3362,3532,3706],Qr={low:[7,10,15,20,26,18,20,24,30,18,20,24,26,30,22,24,28,30,28,28,28,28,30,30,26,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30],medium:[10,16,26,18,24,16,18,22,22,26,30,22,22,24,24,28,28,26,26,26,26,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28],quartile:[13,22,18,26,18,24,18,22,20,24,28,26,24,20,30,24,28,28,26,30,28,30,30,30,30,28,30,30,30,30,30,30,30,30,30,30,30,30,30,30],high:[17,28,22,16,22,28,26,26,24,28,24,28,22,24,24,30,28,28,26,28,30,24,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30]},ei={low:[1,1,1,1,1,2,2,2,2,4,4,4,4,4,6,6,6,6,7,8,8,9,9,10,12,12,12,13,14,15,16,17,18,19,19,20,21,22,24,25],medium:[1,1,1,2,2,4,4,4,5,5,5,8,9,9,10,10,11,13,14,16,17,17,18,20,21,23,25,26,28,29,31,33,35,37,38,40,43,45,47,49],quartile:[1,1,2,2,4,4,6,6,8,8,8,10,12,16,12,17,16,18,21,20,23,23,25,27,29,34,34,35,38,40,43,45,48,51,53,56,59,62,65,68],high:[1,1,2,4,4,4,5,6,8,8,11,11,16,16,18,16,19,21,25,25,25,34,30,32,35,37,40,42,45,48,51,54,57,60,63,66,70,74,77,81]},q={size:{encode:t=>21+4*(t-1),decode:t=>(t-17)/4},sizeType:t=>Math.floor((t+7)/17),alignmentPatterns(t){if(t===1)return[];const e=6,r=q.size.encode(t)-e-1,i=r-e,n=Math.ceil(i/28);let o=Math.floor(i/n);o%2?o+=1:i%n*2>=n&&(o+=2);const s=[e];for(let a=1;a<n;a++)s.push(r-(n-a)*o);return s.push(r),s},ECCode:{low:1,medium:0,quartile:3,high:2},formatMask:21522,formatBits(t,e){const r=q.ECCode[t]<<3|e;let i=r;for(let n=0;n<10;n++)i=i<<1^(i>>9)*1335;return(r<<10|i)^q.formatMask},versionBits(t){let e=t;for(let r=0;r<12;r++)e=e<<1^(e>>11)*7973;return t<<12|e},alphabet:{numeric:De("0123456789"),alphanumerc:De("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:")},lengthBits(t,e){return{numeric:[10,12,14],alphanumeric:[9,11,13],byte:[8,16,16],kanji:[8,10,12],eci:[0,0,0]}[e][q.sizeType(t)]},modeBits:{numeric:"0001",alphanumeric:"0010",byte:"0100",kanji:"1000",eci:"0111"},capacity(t,e){const r=Zr[t-1],i=Qr[e][t-1],n=ei[e][t-1],o=Math.floor(r/n)-i,s=n-r%n;return{words:i,numBlocks:n,shortBlocks:s,blockLen:o,capacity:(r-i*n)*8,total:(i+o)*n+n-s}}},Ae=[(t,e)=>(t+e)%2==0,(t,e)=>e%2==0,(t,e)=>t%3==0,(t,e)=>(t+e)%3==0,(t,e)=>(Math.floor(e/2)+Math.floor(t/3))%2==0,(t,e)=>t*e%2+t*e%3==0,(t,e)=>(t*e%2+t*e%3)%2==0,(t,e)=>((t+e)%2+t*e%3)%2==0],v={tables:(t=>{const e=$(256,0),r=$(256,0);for(let i=0,n=1;i<256;i++)e[i]=n,r[n]=i,n<<=1,n&256&&(n^=t);return{exp:e,log:r}})(285),exp:t=>v.tables.exp[t],log(t){if(t===0)throw new Error(`GF.log: invalid arg=${t}`);return v.tables.log[t]%255},mul(t,e){return t===0||e===0?0:v.tables.exp[(v.tables.log[t]+v.tables.log[e])%255]},add:(t,e)=>t^e,pow:(t,e)=>v.tables.exp[v.tables.log[t]*e%255],inv(t){if(t===0)throw new Error(`GF.inverse: invalid arg=${t}`);return v.tables.exp[255-v.tables.log[t]]},polynomial(t){if(t.length==0)throw new Error("GF.polymomial: invalid length");if(t[0]!==0)return t;let e=0;for(;e<t.length-1&&t[e]==0;e++);return t.slice(e)},monomial(t,e){if(t<0)throw new Error(`GF.monomial: invalid degree=${t}`);if(e==0)return[0];let r=$(t+1,0);return r[0]=e,v.polynomial(r)},degree:t=>t.length-1,coefficient:(t,e)=>t[v.degree(t)-e],mulPoly(t,e){if(t[0]===0||e[0]===0)return[0];const r=$(t.length+e.length-1,0);for(let i=0;i<t.length;i++)for(let n=0;n<e.length;n++)r[i+n]=v.add(r[i+n],v.mul(t[i],e[n]));return v.polynomial(r)},mulPolyScalar(t,e){if(e==0)return[0];if(e==1)return t;const r=$(t.length,0);for(let i=0;i<t.length;i++)r[i]=v.mul(t[i],e);return v.polynomial(r)},mulPolyMonomial(t,e,r){if(e<0)throw new Error("GF.mulPolyMonomial: invalid degree");if(r==0)return[0];const i=$(t.length+e,0);for(let n=0;n<t.length;n++)i[n]=v.mul(t[n],r);return v.polynomial(i)},addPoly(t,e){if(t[0]===0)return e;if(e[0]===0)return t;let r=t,i=e;r.length>i.length&&([r,i]=[i,r]);let n=$(i.length,0),o=i.length-r.length,s=i.slice(0,o);for(let a=0;a<s.length;a++)n[a]=s[a];for(let a=o;a<i.length;a++)n[a]=v.add(r[a-o],i[a]);return v.polynomial(n)},remainderPoly(t,e){const r=Array.from(t);for(let i=0;i<t.length-e.length+1;i++){const n=r[i];if(n!==0)for(let o=1;o<e.length;o++)e[o]!==0&&(r[i+o]=v.add(r[i+o],v.mul(e[o],n)))}return r.slice(t.length-e.length+1,r.length)},divisorPoly(t){let e=[1];for(let r=0;r<t;r++)e=v.mulPoly(e,[1,v.pow(2,r)]);return e},evalPoly(t,e){if(e==0)return v.coefficient(t,0);let r=t[0];for(let i=1;i<t.length;i++)r=v.add(v.mul(e,r),t[i]);return r},euclidian(t,e,r){v.degree(t)<v.degree(e)&&([t,e]=[e,t]);let i=t,n=e,o=[0],s=[1];for(;2*v.degree(n)>=r;){let c=i,d=o;if(i=n,o=s,i[0]===0)throw new Error("rLast[0] === 0");n=c;let l=[0];const f=v.inv(i[0]);for(;v.degree(n)>=v.degree(i)&&n[0]!==0;){const h=v.degree(n)-v.degree(i),g=v.mul(n[0],f);l=v.addPoly(l,v.monomial(h,g)),n=v.addPoly(n,v.mulPolyMonomial(i,h,g))}if(l=v.mulPoly(l,o),s=v.addPoly(l,d),v.degree(n)>=v.degree(i))throw new Error(`Division failed r: ${n}, rLast: ${i}`)}const a=v.coefficient(s,0);if(a==0)throw new Error("sigmaTilde(0) was zero");const u=v.inv(a);return[v.mulPolyScalar(s,u),v.mulPolyScalar(n,u)]}};function ti(t){return{encode(e){const r=v.divisorPoly(t),i=Array.from(e);return i.push(...r.slice(0,-1).fill(0)),Uint8Array.from(v.remainderPoly(i,r))},decode(e){const r=e.slice(),i=v.polynomial(Array.from(e));let n=$(t,0),o=!1;for(let l=0;l<t;l++){const f=v.evalPoly(i,v.exp(l));n[n.length-1-l]=f,f!==0&&(o=!0)}if(!o)return r;n=v.polynomial(n);const s=v.monomial(t,1),[a,u]=v.euclidian(s,n,t),c=$(v.degree(a),0);let d=0;for(let l=1;l<256&&d<c.length;l++)v.evalPoly(a,l)===0&&(c[d++]=v.inv(l));if(d!==c.length)throw new Error("RS.decode: invalid errors number");for(let l=0;l<c.length;l++){const f=r.length-1-v.log(c[l]);if(f<0)throw new Error("RS.decode: invalid error location");const h=v.inv(c[l]);let g=1;for(let m=0;m<c.length;m++)l!==m&&(g=v.mul(g,v.add(1,v.mul(c[m],h))));r[f]=v.add(r[f],v.mul(v.evalPoly(u,h),v.inv(g)))}return r}}}function ri(t,e){const{words:r,shortBlocks:i,numBlocks:n,blockLen:o,total:s}=q.capacity(t,e),a=ti(r);return{encode(u){const c=[],d=[];for(let g=0;g<n;g++){const m=g<i,E=o+(m?0:1);c.push(u.subarray(0,E)),d.push(a.encode(u.subarray(0,E))),u=u.subarray(E)}const l=Pe(c),f=Pe(d),h=new Uint8Array(l.length+f.length);return h.set(l),h.set(f,l.length),h},decode(u){if(u.length!==s)throw new Error(`interleave.decode: len(data)=${u.length}, total=${s}`);const c=[];for(let f=0;f<n;f++){const h=f<i;c.push(new Uint8Array(r+o+(h?0:1)))}let d=0;for(let f=0;f<o;f++)for(let h=0;h<n;h++)c[h][f]=u[d++];for(let f=i;f<n;f++)c[f][o]=u[d++];for(let f=o;f<o+r;f++)for(let h=0;h<n;h++){const g=h<i;c[h][f+(g?0:1)]=u[d++]}const l=[];for(const f of c)l.push(...Array.from(a.decode(f)).slice(0,-r));return Uint8Array.from(l)}}}function ii(t,e,r,i=!1){const n=q.size.encode(t);let o=new D(n+2);const s=new D(3).rect(0,3,!0).border(1,!1).border(1,!0).border(1,!1);o=o.embed(0,s).embed({x:-s.width,y:0},s).embed({x:0,y:-s.height},s),o=o.rectSlice(1,n);const a=new D(1).rect(0,1,!0).border(1,!1).border(1,!0),u=q.alignmentPatterns(t);for(const c of u)for(const d of u)o.isDefined(d,c)||o.embed({x:d-2,y:c-2},a);o=o.hLine({x:0,y:6},1/0,({x:c})=>o.isDefined(c,6)?void 0:c%2==0).vLine({x:6,y:0},1/0,({y:c})=>o.isDefined(6,c)?void 0:c%2==0);{const c=q.formatBits(e,r),d=l=>!i&&(c>>l&1)==1;for(let l=0;l<6;l++)o.set(8,l,d(l));for(let l=6;l<8;l++)o.set(8,l+1,d(l));for(let l=8;l<15;l++)o.set(8,n-15+l,d(l));for(let l=0;l<8;l++)o.set(n-l-1,8,d(l));for(let l=8;l<9;l++)o.set(15-l-1+1,8,d(l));for(let l=9;l<15;l++)o.set(15-l-1,8,d(l));o.set(8,n-8,!i)}if(t>=7){const c=q.versionBits(t);for(let d=0;d<18;d+=1){const l=!i&&(c>>d&1)==1,f=Math.floor(d/3),h=d%3+n-8-3;o.set(h,f,l),o.set(f,h,l)}}return o}function ni(t,e,r){const i=t.height,n=Ae[e];let o=-1,s=i-1;for(let a=i-1;a>0;a-=2){for(a==6&&(a=5);;s+=o){for(let u=0;u<2;u+=1){const c=a-u;t.isDefined(c,s)||r(c,s,n(c,s))}if(s+o<0||s+o>=i)break}o=-o}}function oi(t){let e="numeric";for(let r of t)if(!q.alphabet.numeric.has(r)&&(e="alphanumeric",!q.alphabet.alphanumerc.has(r)))return"byte";return e}function si(t){if(typeof t!="string")throw new Error(`utf8ToBytes expected string, got ${typeof t}`);return new Uint8Array(new TextEncoder().encode(t))}function Le(t,e,r,i,n=si){let o="",s=r.length;if(i==="numeric"){const f=q.alphabet.numeric.decode(r.split("")),h=f.length;for(let g=0;g<h-2;g+=3)o+=K(f[g]*100+f[g+1]*10+f[g+2],10);h%3===1?o+=K(f[h-1],4):h%3===2&&(o+=K(f[h-2]*10+f[h-1],7))}else if(i==="alphanumeric"){const f=q.alphabet.alphanumerc.decode(r.split("")),h=f.length;for(let g=0;g<h-1;g+=2)o+=K(f[g]*45+f[g+1],11);h%2==1&&(o+=K(f[h-1],6))}else if(i==="byte"){const f=n(r);s=f.length,o=Array.from(f).map(h=>K(h,8)).join("")}else throw new Error("encode: unsupported type");const{capacity:a}=q.capacity(t,e),u=K(s,q.lengthBits(t,i));let c=q.modeBits[i]+u+o;if(c.length>a)throw new Error("Capacity overflow");c+="0".repeat(Math.min(4,Math.max(0,a-c.length))),c.length%8&&(c+="0".repeat(8-c.length%8));const d="1110110000010001";for(let f=0;c.length!==a;f++)c+=d[f%d.length];const l=Uint8Array.from(c.match(/(.{8})/g).map(f=>+`0b${f}`));return ri(t,e).encode(l)}function Be(t,e,r,i,n=!1){const o=ii(t,e,i,n);let s=0;const a=8*r.length;if(ni(o,i,(u,c,d)=>{let l=!1;s<a&&(l=(r[s>>>3]>>(7-s&7)&1)!==0,s++),o.set(u,c,l!==d)}),s!==a)throw new Error("QR: bytes left after draw");return o}const Ke=t=>{const e=t.map(r=>r?"1":"0").join("");return{len:e.length,n:+`0b${e}`}},Xe=[!0,!1,!0,!0,!0,!1,!0],Ye=[!1,!1,!1,!1],le=Ke([...Xe,...Ye]),Fe=Ke([...Ye,...Xe]);function ai(t){const{width:e,height:r}=t,i=t.transpose();let n=0;for(let d=0;d<r;d++)t.getRuns(d,l=>{l>=5&&(n+=3+(l-5))});for(let d=0;d<e;d++)i.getRuns(d,l=>{l>=5&&(n+=3+(l-5))});let o=0;for(let d=0;d<r-1;d++)o+=3*t.countBoxes2x2(d);let s=0;for(let d=0;d<r;d++)s+=40*t.countPatternInRow(d,le.len,le.n,Fe.n);for(let d=0;d<e;d++)s+=40*i.countPatternInRow(d,le.len,le.n,Fe.n);let a=0;a=t.popcnt();const u=a/(r*e)*100,c=10*Math.floor(Math.abs(u-50)/5);return n+o+s+c}function ui(t,e,r,i){if(i===void 0){const n=Yr();for(let o=0;o<Ae.length;o++)n.add(ai(Be(t,e,r,o,!0)),o);i=n.get()}if(i===void 0)throw new Error("Cannot find mask");return Be(t,e,r,i)}function li(t){if(!je.includes(t))throw new Error(`Invalid error correction mode=${t}. Expected: ${je}`)}function ci(t){if(!Ue.includes(t))throw new Error(`Encoding: invalid mode=${t}. Expected: ${Ue}`);if(t==="kanji"||t==="eci")throw new Error(`Encoding: ${t} is not supported (yet?).`)}function fi(t){if(![0,1,2,3,4,5,6,7].includes(t)||!Ae[t])throw new Error(`Invalid mask=${t}. Expected number [0..7]`)}function di(t,e="raw",r={}){const i=r.ecc!==void 0?r.ecc:"medium";li(i);const n=r.encoding!==void 0?r.encoding:oi(t);ci(n),r.mask!==void 0&&fi(r.mask);let o=r.version,s,a=new Error("Unknown error");if(o!==void 0)Xr(o),s=Le(o,i,t,n,r.textEncoder);else for(let d=1;d<=40;d++)try{s=Le(d,i,t,n,r.textEncoder),o=d;break}catch(l){a=l}if(!o||!s)throw a;let u=ui(o,i,s,r.mask);u.assertDrawn();const c=r.border===void 0?2:r.border;if(!Number.isSafeInteger(c))throw new Error(`invalid border type=${typeof c}`);if(u=u.border(c,!1),r.scale!==void 0&&(u=u.scale(r.scale)),e==="raw")return u.toRaw();if(e==="ascii")return u.toASCII();if(e==="svg")return u.toSVG(r.optimize);if(e==="gif")return u.toGIF();if(e==="term")return u.toTerm();throw new Error(`Unknown output: ${e}`)}const hi=V("matter",()=>{const t=J(),e=O({fabricCount:0,latestStatus:void 0}),r=O({qrCodeMatrix:null,manualCode:"",availableUntil:null,showModal:!1,expiresInMs:0,timeout:null});async function i(){await t.busyBar.SmartHomePairingGet().then(s=>{e.value.fabricCount=s.fabric_count||0,e.value.latestStatus=s.latest_pairing_status}).catch(async s=>{await T(s,"Couldn't get Matter commissioning status",!0)})}async function n(){await t.busyBar.SmartHomePair().then(s=>{r.value.manualCode=s.manual_code||"",r.value.availableUntil=new Date(Number(s.available_until)),r.value.timeout&&clearTimeout(r.value.timeout),r.value.expiresInMs=r.value.availableUntil.getTime()-Date.now(),r.value.qrCodeMatrix=di(s.qr_code,"raw"),r.value.timeout=setTimeout(()=>{r.value.showModal=!1,r.value.qrCodeMatrix=null,r.value.manualCode="",r.value.availableUntil=null,r.value.timeout=null,console.debug("Matter commissioning link expired")},r.value.expiresInMs),r.value.showModal=!0}).catch(async s=>{await T(s,"Couldn't request Matter commissioning link")})}async function o(){await t.busyBar.SmartHomeErase().then(()=>{console.debug("All Matter pairings deleted, waiting for device to reboot")}).catch(async s=>{console.error(s.message),!s.message.includes("timed out")&&await T(s,"Couldn't delete pairings")})}return{matterCommissioning:e,matterLink:r,fetchMatterCommissioning:i,requestMatterLink:n,deleteAllPairings:o}}),pi=V("timezone",()=>{const t=J(),e=O(void 0);async function r(){return await t.busyBar.TimeTimezoneGet().then(o=>(e.value=o.name,o.name)).catch(async o=>(await T(o,"Couldn't get timezone",!0),e.value))}async function i(n){return await t.busyBar.TimeTimezoneSet({timezone:n}).then(()=>(e.value=n,!0)).catch(async o=>(await T(o,"Couldn't set timezone"),!1))}return{timezone:e,fetchTimezone:r,setTimezone:i}}),mi=V("screenStream",()=>({currentFrame:O(null)}),{persist:!1});function pe(t){return!!t&&typeof t=="object"&&!Array.isArray(t)&&!(t instanceof Uint8Array)}function yi(t){return pe(t)&&"from"in t&&"to"in t&&Object.keys(t).length===2}function ce(t){if(typeof t=="string")return t;if(t===void 0)return"undefined";if(t===null)return"null";if(typeof t=="number"||typeof t=="boolean"||typeof t=="bigint")return String(t);try{return JSON.stringify(t)}catch{return String(t)}}function Ee(t,e){if(t!==e){if(pe(t)&&pe(e)){const r=new Set([...Object.keys(t),...Object.keys(e)]),i={};for(const n of r){const o=Ee(t[n],e[n]);o!==void 0&&(i[n]=o)}return Object.keys(i).length?i:void 0}if(Array.isArray(t)&&Array.isArray(e)){const r=Math.max(t.length,e.length),i=[];let n=!1;for(let o=0;o<r;o++){const s=Ee(t[o],e[o]);s!==void 0&&(i[o]=s,n=!0)}return n?i:void 0}return{from:t,to:e}}}function _e(t,e=""){return t===void 0?[]:yi(t)?[`${e||"value"}: ${ce(t.from)} -> ${ce(t.to)}`]:Array.isArray(t)?t.flatMap((r,i)=>_e(r,`${e}[${i}]`)):pe(t)?Object.entries(t).flatMap(([r,i])=>{const n=e?`${e}.${r}`:r;return _e(i,n)}):e?[`${e}: ${ce(t)}`]:[ce(t)]}function vi(t){switch(t){case C.WifiSecurity.OPEN:return"Open";case C.WifiSecurity.WPA:return"WPA";case C.WifiSecurity.WPA2:return"WPA2";case C.WifiSecurity.WEP:return"WEP";case C.WifiSecurity.WPA_WPA2:return"WPA/WPA2";case C.WifiSecurity.WPA3:return"WPA3";case C.WifiSecurity.WPA2_WPA3:return"WPA2/WPA3";default:return}}function gi(t){switch(t){case C.IpConfigurationMethod.DHCP:return"dhcp";case C.IpConfigurationMethod.STATIC:return"static";default:return}}function bi(t){switch(t){case C.IpProtocol.IPV4:return"ipv4";case C.IpProtocol.IPV6:return"ipv6";default:return}}function wi(t){if(t){if(t.includes(":"))return"ipv6";if(t.includes("."))return"ipv4"}}function Ei(t){switch(t){case C.MatterCommissioningStatus.NEVER_STARTED:return"never_started";case C.MatterCommissioningStatus.STARTED:return"started";case C.MatterCommissioningStatus.COMPLETED_SUCCESSFULLY:return"completed_successfully";case C.MatterCommissioningStatus.FAILED:return"failed";default:return}}const _i=V("stateStream",()=>{const t=te(),e=J(),r=Hr(),i=Jr(),n=we(),o=hi(),s=pi(),a=Ze(),u=mi(),c=me().public.barUrl||window.location.origin,d=O(!1),l=O(!1),f=Me(new Gr({addr:c,token:t.apiKey||""},{timeout:5e3,dataTimeout:2e3})),h=O(null),g=O(!0);function m(b){const y=b.name;y&&(e.deviceName=y)}function E(b){const y=b.known,I=y?{state:y.batteryStatus?C.BatteryStatus[y.batteryStatus]:void 0,battery_charge:y.batteryChargePercent??0,battery_voltage:y.batteryVoltageMv??0,battery_current:y.batteryCurrentMa??0,usb_voltage:y.usbVoltageMv??0}:void 0;I&&(e.deviceStatus={...e.deviceStatus,power:I})}function _(b){if(b.automatic){i.displayBrightness={value:"auto"};return}const y=b.manual?.brightness;y!==void 0?i.displayBrightness={value:y}:i.displayBrightness={value:0}}function p(b){const y=b.volume;y!==void 0?r.audio={volume:y??0}:r.audio={volume:0}}function w(b){const y=Object.keys(b).filter(W=>W!=="ipAddresses")[0],I=b.connected,U=(Array.isArray(b.ipAddresses)?b.ipAddresses:[]).find(W=>W.address),M=a.wifi?.ip_config,S=U?U.address:void 0;let Z=y;y==="connected"&&I?.status===C.WifiConnectionStatus.RECONNECTING&&(Z="reconnecting");const k={state:Z,ssid:I?.ssid,bssid:I?.bssid,channel:I?.channel,rssi:I?.rssi,security:vi(I?.security),ip_config:U?{ip_method:gi(U.method)??M?.ip_method,ip_type:bi(U.protocol)??wi(S)??M?.ip_type,address:S,gateway:U.gateway,mask:U.netmask}:void 0},N=a.wifi?.state;a.wifi=k,N!==k.state&&(k.state==="connected"?window.dispatchEvent(new Event("wifi-reconnected")):N==="connected"&&window.dispatchEvent(new Event("wifi-disconnected")))}function P(b){if(n.autoUpdate.isChecking=!1,b.available){n.autoUpdate.status="available",n.autoUpdate.availableVersion=b.available?.version??null,n.autoUpdate.isAllowed=!0;return}const y=b.unavailable?.reason;if(n.autoUpdate.availableVersion=null,y===he.CheckError.NOT_AVAILABLE){n.autoUpdate.status="not_available",n.autoUpdate.isAllowed=!0;return}if(y===he.CheckError.FAILURE){n.autoUpdate.status="failure",n.autoUpdate.isAllowed=!1;return}n.autoUpdate.status=null}function B(b){b.name&&(s.timezone=b.name)}function F(b){const y=b.state;o.matterCommissioning={fabricCount:b.fabricCount??0,latestStatus:y?{value:Ei(y.status),timestamp:y.timestamp??0}:void 0}}function G(b){if(b.updates)for(const y of b.updates)switch(y.state){case"deviceName":y.deviceName&&m(y.deviceName);break;case"power":y.power&&E(y.power);break;case"brightness":y.brightness&&_(y.brightness);break;case"audioVolume":y.audioVolume&&p(y.audioVolume);break;case"wifi":y.wifi&&w(y.wifi);break;case"updateCheck":y.updateCheck&&P(y.updateCheck);break;case"timezone":y.timezone&&B(y.timezone);break;case"matter":y.matter&&F(y.matter);break;case"frame":y.frame&&(u.currentFrame=y.frame);break}}async function A(b){const y=h.value;if(y===null){h.value=b,console.debug("[state stream status] Initial stream status:",b);return}const I=Ee(y,b);if(I)for(const H of _e(I))console.debug("[state stream status]",H);h.value=b,h.value.data.status===Y.STALE&&y?.data.status!==Y.STALE&&g.value&&(console.debug("No state messages received for a while, checking connection..."),e.setRefreshInterval(),await e.checkConnection()===!1&&f.value.stop())}return{showStateStreamFailBanner:d,showResourceLimitErrorBanner:l,streamStatus:h,stream:f,doCheckConnectionOnStreamDataStale:g,applyStateMessage:G,applyStreamStatus:A}}),Ze=V("wifi",()=>{const t=J(),e=_i(),r=O(void 0);async function i(){return await t.busyBar.WifiStatusGet().then(u=>(r.value=u,u)).catch(async u=>(await T(u,"Couldn't fetch WiFi state",!0),r.value))}async function n(){const a=t.refreshInterval;return a&&t.clearRefreshInterval(),e.doCheckConnectionOnStreamDataStale=!1,await t.busyBar.WifiNetworksGet({timeout:45e3}).then(u=>{if(!u||!Array.isArray(u.networks))throw new Error("Failed to fetch WiFi networks");return u.networks=u.networks.reduce((c,d)=>{const l=c.find(f=>f.ssid===d.ssid);if(!l)c.push(d);else if(d.rssi&&l.rssi&&d.rssi<l.rssi){const f=c.indexOf(l);c[f]=d}return c},[]),u.networks}).catch(async u=>{if(r.value?.state!=="connected")return await T(u,"Couldn't list WiFi networks",!1,0),[]}).finally(()=>{e.doCheckConnectionOnStreamDataStale=!0,a&&t.setRefreshInterval()})}async function o(a){const u=t.refreshInterval;return u&&t.clearRefreshInterval(),e.doCheckConnectionOnStreamDataStale=!1,await t.busyBar.WifiConnect({...a,timeout:45e3}).catch(async c=>(await T(c,"Couldn't connect to WiFi network",!1,0),!1)).finally(()=>{e.doCheckConnectionOnStreamDataStale=!0,u&&t.setRefreshInterval()})}async function s(){const a=t.refreshInterval;return a&&t.clearRefreshInterval(),e.doCheckConnectionOnStreamDataStale=!1,await t.busyBar.WifiDisconnect().catch(async u=>(await T(u,"Couldn't disconnect from WiFi network",!1,0),!1)).finally(()=>{e.doCheckConnectionOnStreamDataStale=!0,a&&t.setRefreshInterval()})}return{wifi:r,fetchWifiState:i,listWifiNetworks:n,connectToWifiNetwork:o,disconnectFromWifiNetwork:s}});export{Ti as R,z as U,J as a,we as b,Hr as c,Jr as d,pi as e,hi as f,te as g,T as h,mi as i,_i as j,Ne as k,Ie as l,x as m,j as n,be as s,L as t,Ze as u};
