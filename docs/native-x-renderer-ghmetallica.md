# NativeXRenderer in Guitar Hero: Metallica

The game's side of NativeXRenderer (`native_x_renderer/`, the library that hooks a game's Direct3D calls and draws
on the host): the map of the XDK Direct3D library linked into `default.xex`, the device and object layouts, the
engine's use of them, and the verification. The hooks and the profile are in
[`src/native_x_renderer_ghmetallica.cpp`](../src/native_x_renderer_ghmetallica.cpp); the library's checklist is
`native_x_renderer/docs/integrating-a-game.md`.

**Status (2026-10-24):** steps 1-4 done. The map comes from static analysis (this executable matched function by
function against POTCAWE's library, XDK 5426, the nearest mapped version, then every function used here read); the
observer matches 100 % of the draws on the title screen, the main menu and in songs (section 7). With
`native_x_renderer = true` the menus, the career's opening cinematic and its songs draw as on the Xenos path at 60 FPS
(since NativeXRenderer's D95), with SMAA or FXAA on the scene, filling ultrawide screens, at about half the GPU
time of the Xenos path at 1440p and above. GH Metallica Launcher has the renderer row, with the Xenos path the
default until a full play test (`kNativeXRendererDefault` in `launcher/game.h`).

**Main findings:**
- **The device layout is Dante's Inferno's up to the end of the register shadow** (POTCAWE's + 128): six dirty
  masks at `+0`…`+40`, the command buffer at `+48`/`+52`/`+56`, fetch constants `+1152`, VS/PS constants
  `+1920`/`+6016`, bool/loop `+10112`, the shadow groups at Dante's offsets, the fence counter `+10908`. The block of
  current objects (targets, shaders, viewport, predication, tiling) is POTCAWE's + 144. The resource objects are
  POTCAWE's exactly.
- **The library is not inlined into the engine** as Kameo's is: EndVertices, the typed Lock/Unlock thunks,
  CreateDevice and ClearF are real functions. The engine writes constants and sampler states into the device
  itself, and the setters take the dirty mask or bit from their caller as an extra argument.
- **This XDK has DrawIndexedVerticesUP** (`sub_821C19A0`), which calls BeginIndexedVertices and inlines the end;
  NativeXRenderer has no call for it, so the profile names the inline draw end (`inline_draw_end = 13444`) and
  EndVertices is not hooked.
- **EndTiling resolves the tiles itself**: the engine passes it a destination texture and flags `0x300`, the first
  game to do so (open question 2).

## Contents

1. [The executable](#1-the-executable)
2. [The D3D entry points](#2-the-d3d-entry-points)
3. [The device layout (XDK 2.0.7978)](#3-the-device-layout-xdk-207978)
4. [Resource objects](#4-resource-objects)
5. [Engine behavior](#5-engine-behavior)
6. [Differences from POTCAWE (5426) and Dante's Inferno](#6-differences-from-potcawe-5426-and-dantes-inferno)
7. [Verification](#7-verification)
8. [Open questions](#8-open-questions)

## 1. The executable

- `default.xex`, title ID `41560830`, CRC32 `A7C93B87`, XDK 2.0.7978. The library confirms the version itself: the
  engines init `sub_821CFB18` passes `0x1F2A0003` (`lis r3,7978; ori r3,r3,3`, `0x821CFB38`/`0x821CFB4C`) to
  `VdInitializeEngines` (`0x821CFB50`).
- **Layout around the library** (found from the PM4 cluster, then the POTCAWE matches and the callers):
  - `0x821B0000`–`0x821BC000`: an engine 2D module (*likely* text/sprites): it calls DrawVerticesUP
    (`sub_821B47C0`, `sub_821B4FB0`, `sub_821B5EC8`, `sub_821B69B0`, `sub_821B79F8`), the only engine
    BeginVertices/EndVertices pair (`sub_821B2ED8`), VB/IB locks (`sub_821B72C8`), and writes constants inline.
  - `0x821BD8E0`–`0x821BF400`: XGraphics-style helpers: texture/surface headers (`sub_821BEEA0` texture header,
    `sub_821BF198` XGSetSurfaceHeader, POTCAWE `sub_822ABE40`, 0.83), tile math (`sub_821BDA58`, POTCAWE
    `sub_822AA730`, 1.00), TextureUnlock (`sub_821BDA28`, POTCAWE `sub_822AA700`, 1.00).
  - **`0x821BF408`–`0x821DD6F0`: the D3D device code**, about 560 functions, the draws inside it
    (`0x821C0F98`–`0x821C2278`, unlike POTCAWE where they sit apart). Roughly: resource creation
    (`sub_821BF690` CreateTexture-like, 47 sites; `sub_821BF7B0` surface creation through XGSetSurfaceHeader, 58
    sites), SetTexture, constant and shader setters, the draws, render-state setters (`0x821C2570`–`0x821C3E40`),
    setters for targets/viewport/scissor, locks, the command buffer (allocator, kick, new segment, fences, waits),
    the flush helpers, the program flush, queries, device init, tiling, Swap, Present, the tiling replay worker,
    Present's internal draws, the clears and Resolve.
  - `0x821DD700`–`0x821DF7E0`: a texture-from-file loader (`sub_821DE560` → `sub_821DDFD8`), then libjpeg
    (`sub_821DF7E0`) and libpng (`sub_821DFA00`) (the setjmp callers in `ghmetallica_functions.toml`).
- **The device pointer** is `[0x82AA1574]`: the engine's renderer block `0x82AA0C80` (`lis r11,-32086; addi
  r31,r11,3200`) + 2292, read as `lwz r3,2292(r31)` everywhere (1609 sites). The renderer init `sub_822E21E8` (from
  `sub_822E4460`) calls **Direct3D_CreateDevice `sub_821C5FE0`**`(0, 1, 0, 1, presentParams, &block[2292])`
  (`0x822E246C`–`0x822E248C`), which allocates a **24064-byte** device, 128-byte aligned (`li r3,24064; li r4,128`,
  `0x821C5FF8`/`0x821C5FFC`) and calls the device init `sub_821CFC20` (POTCAWE `sub_822B6A60`, 0.70).
- **Imports**: `VdSwap` thunk `0x827FBA1C`, called only from Present (`0x821D167C`); `VdPersistDisplay` `0x827FBA0C`
  (`0x821D194C`); `VdGetSystemCommandBuffer` `0x827FBA2C` (`0x821D1618`); `VdInitializeEngines` `0x827FB9CC`.
- **A bare `blr` with 979 callers** is `sub_820E0010`, outside the library (debug output compiled out): never hook
  it. There is none inside the library.

## 2. The D3D entry points

Signatures are `(dev = r3, …)`; a float argument takes its integer slot (as in `calls.inc`). "Engine" counts `bl`
sites outside the library, with the number of distinct functions. Confidence: **known** = the packets or device
fields leave no doubt; **likely** = structural evidence only.

### Draws

Every draw runs the same flush inline (masks `+0`…`+40`, section 3), then writes `VGT_INDX_OFFSET` (`0x2102`) **straight
into the command buffer** (`li r11,8450` + the value, not the shadow: rule 2) and a bin-predicated `DRAW_INDX`
(`0xC0032201` with an index buffer, `0xC0012201` auto-indexed); draws over 65535 indices are split. When the tiling
targets are bound (`+10940 & 0x01`) the draw is wrapped in `SET_BIN_MASK_LO(0xFFFFFFFF)`, the draw,
`SET_BIN_MASK_LO(0x80000000)`, `EVENT_WRITE_EXT` (26, an extents record in the buffer `+13248`) and `EVENT_WRITE`
(25), as in GRAW/Kameo but with the `_LO` opcode (`0x60`).

| Address | Call | Evidence | Engine | Confidence |
|---|---|---|---|---|
| `sub_821C1E30` | `DrawIndexedVertices(dev, prim, baseVertex, startIndex, indexCount)` | Flush `0x821C1E40`–`0x821C2068`; `VGT_INDX_OFFSET` = r5 (`0x821C2098`–`0x821C20A8`); index buffer `[+12444]` (`0x821C2070`): base `[ib+24]` (`0x821C20D4`), 32-bit indices when `[ib+0]` bit 31 (`0x821C20E8`, `0x821C2114`); `DRAW_INDX` 4 dwords (`0x821C2164`). POTCAWE `sub_824AC3C0`, 0.76. | 6 in 5: `sub_824145E8`, `sub_824146E0`, `sub_824147E8`, `sub_82414F18` ×2, `sub_8247B998` | known |
| `sub_821C1A40` | `DrawVertices(dev, prim, startVertex, vertexCount)` | Same flush; `VGT_INDX_OFFSET` = r5 (`0x821C1C9C`–`0x821C1CAC`); `DRAW_INDX` 2 dwords (`0x821C1D00`–`0x821C1D24`). POTCAWE `sub_824ABFD8`, 0.75. | 11 in 7 | known |
| `sub_821C0F98` | `BeginVertices(dev, prim, vertexCount, stride)` → vertex pointer (0 on failure) | Stream-0 stride byte `+12536` set to `stride/4` around the flush (`0x821C0FB4`, `0x821C121C`), patch bit `+16 |= 0x80000` if a VS is bound and the stride differs (`0x821C0FD0`–`0x821C0FE4`); fetch slot 31's dirty bit cleared before the flush (`rldicr 0,62`, `0x821C0FF0`) and set again after (`0x821C1410`); vertex memory from the allocator `sub_821C82F0(dev, dwords, 16)` (`0x821C1240`); writes `VGT_INDX_OFFSET` = 0, fetch slot 31 as type-0 `0x000548BA` (vertex fetch 95 = dwords 4–5: address \| 3, size), three zeros to `0x5000`, and `DRAW_INDX` 2 dwords **after the write pointer without moving it**; saves the end of its packets at `+13444`, the data at `+13448`, the dword count at `+13456` (`0x821C1418`–`0x821C1420`). POTCAWE `sub_824ABAD8`, 0.77. | 1: `sub_821B2ED8` (`0x821B30CC`), plus DrawVerticesUP | known |
| `sub_821C1488` | `BeginIndexedVertices(dev, prim, baseVertex, numVertices, indexCount, indexFormat, stride, ppIndexData = r10, ppVertexData = stack +84)` → HRESULT | Like BeginVertices with a second allocation for the indices (`0x821C1748`, `0x821C175C`); `VGT_INDX_OFFSET` = baseVertex (`0x821C1794`); `DRAW_INDX` 4 dwords (`0x821C1864`); saves `+13444` (end), `+13448`/`+13452` (vertex/index data), `+13456`/`+13460` (dword counts) (`0x821C1960`–`0x821C1970`); 32-bit indices when `indexFormat & 4`. Returns `0x8007000E` on failure. No POTCAWE counterpart (GRAW `sub_824C48C0`). | 0: only DrawIndexedVerticesUP | known |
| `sub_821C1430` | `EndVertices(dev)` | `lwz r11,13444(r3); stw r11,48(r3); blr` — the whole function. | 1: `sub_821B2ED8` (`0x821B3498`), with no D3D call between it and the BeginVertices at `0x821B30CC` | known |
| `sub_821C1440` | `DrawVerticesUP(dev, prim, vertexCount, data, stride)` | BeginVertices (`0x821C1460`), `memcpy` (`sub_8252C9B0`, `0x821C1474`), inlined end `[+48] = [+13444]` (`0x821C1478`/`0x821C147C`). POTCAWE `sub_824ABF78`, 0.79. | 31 in 27 | known |
| `sub_821C19A0` | `DrawIndexedVerticesUP(dev, prim, minVertexIndex, numVertices, indexCount, indexData, indexFormat, vertexData, stride = stack +84)` | BeginIndexedVertices with baseVertex = −minVertexIndex (`neg r5,r5`, `0x821C19D4`; `0x821C19E4`), two `memcpy`, inlined end `[+48] = [+13444]` (`0x821C1A30`/`0x821C1A34`). **No `calls.inc` id.** | 4 in 2: `sub_8244C968` ×3, `sub_824D5AA0` | known |

No other draw writer is reachable from the engine: `sub_821C5B20` (`DRAW_INDX`) and `sub_821C0410` (`DRAW_INDX_2`)
build buffers at device init (`sub_821CFC20` only); `sub_821C64E0`/`sub_821C6638`, `sub_821D3DB0` and
`sub_821D9D08` are Present's internal draws (through `sub_821D6680` and `sub_821DA978`); `sub_821DAB50` is the
clear's aliased-surface draw; Resolve has its own.

### Render targets, clears, resolves, tiling, predication, viewport, scissor

| Address | Call | Evidence | Engine | Confidence |
|---|---|---|---|---|
| `sub_821C5308` | `SetRenderTarget(dev, index, surface)` | Slot `+12448 + 4·index` (`addi r11,r4,3112`, `0x821C5314`–`0x821C5324`); `[surface+28]` → RB_COLOR_INFO `+10372` (RT1–3: `+10380…`, registers `0x2003`–`0x2005`), dirty bit `57 − (register − 0x2000)` of `+16`: RT0 bit 56, RT1–3 bits 54–52
(`0x821C533C`–`0x821C5380`); for RT0 `sub_821C5278` (RB_SURFACE_INFO `+10368` ← `[surface+24]`, PA_SC_AA_CONFIG `+10684`, default scissor, then SetViewport); recomputes `+10940 & 0x01`; RB_COLOR_MASK `+10460` from the per-RT masks `+11868…+11880`. **Alias:** the engine calls `sub_821C5930` (`b 0x821c5308`). POTCAWE `sub_822B4F60`, 0.63. | 0 direct; **75 through the alias** | known |
| `sub_821C5658` | `SetDepthStencilSurface(dev, surface)` | `+12464` (`0x821C5670`); RB_DEPTH_INFO `+10376` ← `[surface+28]` (`0x821C5690`/`0x821C5694`); RB_HIZCONTROL `+10560` (`0x2203`) ← `[surface+32]` (`0x821C56A4`/`0x821C56A8`); `+10942` bit 5 from the HiZ word; RB_SURFACE_INFO through `sub_821C5278` when no RT0; depth/stencil enables into RB_DEPTHCONTROL `+10548` from `+11884`/`+11888`. POTCAWE `sub_822B52C8`, 0.71. | 52 in 30 | known |
| `sub_821DBE18` | `Clear(dev, count, rects, flags, D3DCOLOR, Z = f1, stencil = r9)` | Unpacks the color (`vupkd3d128`, `0x821DBE50`) and calls ClearF once per rect or once with none (`0x821DBE88`, `0x821DBEB8`), stencil passed through. POTCAWE `sub_822C6AA0`, 0.72. | 27 in 21 | known |
| `sub_821DBCF0` | `ClearF(dev, flags, rect, float4 color*, Z = f1, stencil = r8)` | A NULL rect becomes the bound target's size (`[surface+36]`, or the tiled size `+13188/+13192`, `0x821DBD08`–`0x821DBDF8`), then the clear body `sub_821DB640` (`0x821DBE00`): clips to the viewport `+12656…`, `IM_LOAD_IMMEDIATE` ×2, SQ_PROGRAM_CNTL written directly, the aliased-surface fast clear `sub_821DB0D8` → `sub_821DAE38` → `sub_821DAB50` (writes RB_SURFACE_INFO/RB_COLOR_INFO in the packet, fetch slot 0, `DRAW_INDX_2`; POTCAWE `sub_822C5830`, 0.80). POTCAWE `sub_822C6978`, 0.74. | 0: called by Clear and BeginTiling only | known |
| `sub_821DC640` | `Resolve(dev, flags, srcRect, destTexture, destPoint, level, slice, clearColor, Z = f1, stencil = stack +84, params = stack +92)` | Flags bits 0–2 RT0–3 / 4 = depth (`0x821DC650`, `0x821DC688`–`0x821DC6A0`), the sample select from `[surface+24]` (`0x821DC6A4`); destination fetch constant `dest+28…+48` (`0x821DC6F0`–`0x821DC744`); RB_COPY_CONTROL/DEST_BASE/DEST_PITCH/DEST_INFO into the shadow `+10776…+10788` (`0x821DC99C`–`0x821DC9C0`) and written from `0x2318` (`0x821DD478`); `EVENT_WRITE`, `IM_LOAD_IMMEDIATE`, SQ_PROGRAM_CNTL, fetch slot 0 directly, bin-predicated `DRAW_INDX_2`, `WAIT_REG_MEM`. POTCAWE `sub_822C2048`, 0.72. | 74 in 42 (plus EndTiling ×2 and Present's `sub_821D5070`) | known |
| `sub_821D03E0` | `BeginTiling(dev, flags, count, tileRects, clearColor, Z = f1, stencil = r9)` | Count → `+12756` (`0x821D03FC`); rects → `+12760 + 16·i` (`0x821D0430`–`0x821D0470`), 32-aligned (x1, y1) → `+13000 + 8·i`; max extents → `+13188/+13192`, flags → `+13196` (`0x821D04B0`–`0x821D04BC`); unless `flags & 1`, ClearF(dev, `0x3F`, the whole area, clearColor, Z, stencil) with the viewport/scissor saved and restored (`0x821D052C`–`0x821D05BC`); clear color/Z/stencil → `+13200`/`+13216`/`+13220`; reference surface (RT0 else depth) → `+12732`, RT0–3 → `+12736…+12748`, depth → `+12752`, `+12728 = 2`, `+10940 |= 0x20` (`0x821D05E0`–`0x821D0638`); predication reset (`+12716 = −1`); tiling header word `0x86000000` into `+13368`; `EVENT_WRITE(25)`; SetViewport(default); `sub_821C4E78(dev, 1)` (`SET_BIN_*` LO/HI). POTCAWE `sub_822BCC58`, 0.51 (it is the full 5426 form, with the saved targets). | 1: `sub_8243D298` (`0x8243D354`) | known |
| `sub_821D0750` | `EndTiling(dev, resolveFlags, resolveRects, destTexture, clearColor, Z = f1, stencil = r9, params = r10)` | With a destination: per tile, SetPredication(`3 << 2i`) (`0x821D0838`), then **Resolve** of the tile's rect (NULL rects → `+12760`, `0x821D0798`) into destTexture at the tile's corner, with `0x100`/`0x200` (clear RT0/depth with the tiling clear values `+13200…+13220`) on every tile but the last, which uses the caller's clear values (`0x821D08B0`, `0x821D08E0`); SetPredication(0); with more than one tile SetPredication(`0x80000000`), a GPU callback (`sub_821C94E8`, callback `sub_821D2148`), SetPredication(0), kick; header words; clears `+10940 & 0x20`, `+12732`, `+12724`; replay submission `sub_821D30C0(dev, tiles, mask)` (`0x821D0BC0`); SetViewport(default); returns `0x8007000E` on failure. POTCAWE `sub_822BD1E0`, 0.77. | 1: `sub_8243D3D8` (`0x8243D504`), **flags `0x300` with a destination texture** | known |
| — | BeginZPass / EndZPass | Not linked: after EndTiling come `sub_821D0C08`, `sub_821D0C78`, the swap callback and Swap (POTCAWE has them right after EndTiling). | — | known |
| `sub_821D0270` | `SetPredication(dev, mask)` | `+12716` = mask (or `−1` for 0), `+12724` = mask ≠ 0, `+12720 = 0`, `+10932 = mask`, `+10936 = 0` (`0x821D0294`–`0x821D03A4`); recomputes `+10940 & 0x01`; `SET_BIN_MASK_LO(mask)` (`0xC0006000`, `0x821D03B4`). POTCAWE `sub_822BCAE8`, 0.76. | 0: EndTiling ×4, Swap | known |
| `sub_821C51D8` | `SetViewport(dev, D3DVIEWPORT9*)` | Converts `{X, Y, W, H}` (u32) to floats and calls the float setter `sub_821C4F70(dev, x, y, w, h, minZ, maxZ in f1–f6)` (`0x821C5244`), which clips to the bound target (`[surface+36]`, or `+13188/+13192` when tiling) and stores 6 floats at `+12656…+12676` (`0x821C50F0`–`0x821C5138`), `+12680 = 0`, calls SetScissorRect(dev, dev+12684) (`0x821C513C`) and writes PA_CL_VPORT_* `+10504…+10524` (`0x210F`–`0x2114`) with dirty bits 26–21 of `+16`. `sub_821C5258` is the D3DVIEWPORTF9 form (BeginTiling only). POTCAWE `sub_822B4E28`, 0.79. | 5 in 5 (plus the library's defaults) | known |
| `sub_821C4838` | `SetScissorRect(dev, rect*)` | `+12684…+12696` (`0x821C48A4`–`0x821C48B8`); intersected with the viewport if `+11864` (scissor test enable, setter `sub_821C51C8`, 18 engine sites); PA_SC_WINDOW_SCISSOR_TL/BR `+10436/+10440`, written through `sub_821C2280` (bin-predicated per tile). POTCAWE `sub_822B4550`, 0.56. | 14 in 6 | known |

### Shaders, vertex declaration, stream source, indices, textures, constants

| Address | Call | Evidence | Engine | Confidence |
|---|---|---|---|---|
| `sub_821C0998` | `SetVertexShader(dev, shader)` | `+12704` (`0x821C0A38`); `+16 |= 0x80000`; old shader fenced; `+10942 &= 0x7F`; literal block `vs + 872 + [vs+892]`: its u64 group mask is cleared from the VS dirty mask `+0` (the program flush loads them with `LOAD_ALU_CONSTANT`), bool dirty bit 56 of `+32` if `[block+8]`, records `{u16 offset, u16 count}` copied into `dev + 1152 + offset` (`0x821C0A48`–`0x821C0B58`). POTCAWE `sub_822B7E18`, 0.80. | 2 (`sub_824443B0` unbinds; `sub_824A8940`, the engine's material binder), plus the library's `sub_821C5E28` | known |
| `sub_821C07D8` | `SetPixelShader(dev, shader)` | `+12700` (`0x821C085C`); `+16 |= 0x100000 | 0x20000`; literal block `ps + 40 + [ps+60]` cleared from `+8`, records copied likewise. POTCAWE `sub_822B7B60`, 0.80. | 3 (`sub_824443B0`, `sub_824A8940`, `sub_8244C968`) | known |
| `sub_821C0B68` | `SetVertexDeclaration(dev, declaration)` | `stw r4,11824(r3)`; `+16 |= 0x80000`. POTCAWE `sub_822B7FE8`, 0.88. | 29 in 27 | known |
| `sub_821C49A8` | `SetStreamSource(dev, stream, buffer, offset, stride, dirtyBit = r8)` | Writes vertex fetch `95 − stream` itself: dword 0 = `[vb+24] + offset` (physical, type bits kept) at `+1912 − 8·s`, dword 1 = `[vb+28] − offset` at `+1916 − 8·s` (`0x821C49CC`–`0x821C4A08`); `+24 |= r8` (the caller's bit: 1 = slot 31 for streams 0–2); buffer `+12468 + 4·s` (`0x821C4A18`, `0x821C4A94`); stride byte `+12536 + s` (`0x821C4AA0`) and `+16 |= 0x80000` if it differs from `+11840 + s`. POTCAWE `sub_822B46A0`, 0.74. | 31 in 13 | known |
| `sub_821C4B50` | `SetIndices(dev, buffer)` | `+12444` (`0x821C4BD4`); old buffer fenced (`[ib+8] = [+10908]`). POTCAWE `sub_822B47C0`, 0.76. | 10 in 6 | known |
| `sub_821BFA10` | `SetTexture(dev, sampler, texture, dirtyBit = r6)` | Pointer `+12552 + 4·i` (`addi r11,r4,3138`, `0x821BFA1C`, `0x821BFB14`); copies the fetch constant `texture+28…+48` into slot i at `+1152 + 24·i` (`addi r11,r4,48; mulli 24`, `0x821BFA38`–`0x821BFACC`), keeping the sampler bits; per-sampler bytes `+11958 + i`, `+11984 + i`; `+24 |= r6` (`0x821BFB00`–`0x821BFB10`; the engine passes `0x80000000 >> i`, `sub_824A8940` `0x824A8DBC`). POTCAWE `sub_822AC668`, 0.69. | 135 in 42 | known |
| `sub_821C0550` | `SetVertexShaderConstantF(dev, start, data, count, dirtyMask = r7)` | `+1920 + 16·start` (`addi r10,r4,120` ×16); `+0 |= r7` (`0x821C0618`–`0x821C0620`). POTCAWE `sub_822B7848` (0.51: 5426 computes the mask itself). | 47 in 20 | known |
| `sub_821C0628` | `SetPixelShaderConstantF(dev, start, data, count, dirtyMask = r7)` | `+6016 + 16·start` (`addi r10,r4,376`); `+8 |= r7`. | 49 in 21 | known |
| `0x821C2570`–`0x821C3E40` | Render-state setters | One function per state, called by the engine directly (RB_DEPTHCONTROL setters with 110, 57, 44 sites…, 95 named with `rs_setters.py`). They write only the shadow. | — | known |
| `sub_821CB180` | Program flush (internal) | Called by the draws when `+16 & 0x1E0000`; patches/loads the shaders, writes **SQ_PROGRAM_CNTL/SQ_CONTEXT_MISC as a type-0 packet straight into the command buffer** from the shadow `+10528/+10532` (`0x821CB86C`–`0x821CB890`), SQ_INTERPOLATOR_CNTL into the shadow `+10536` (`0x821CB3D0`); **shader literals with `LOAD_ALU_CONSTANT`** through `sub_821CADA8` (`0x821CAEB0`; an inline type-0 copy only in the `+10940 & 0x80` mode). POTCAWE `sub_822B1090`, 0.67. | — | known |
| `sub_821C0CF0` | `SetShaderGPRAllocation(dev, flags, vertexGprs, pixelGprs)` | `+10920` = packed GPRs (defaults 64/64, `0x821C0D14`); three `INDIRECT_BUFFER`s (`0xC0013F00`, bin-predicated when `+12724 & flags`) to buffers prebuilt at device init (`+13784/+13788`, `+13796 + 8·n`, `+14816/+14820`). POTCAWE `sub_822B8120`, 0.79. | 2: `sub_823A3DD8`, `sub_823A4588` | known |

### Resources, command buffer, queries, frame

| Address | Call | Evidence | Engine | Confidence |
|---|---|---|---|---|
| `sub_821C7410` | `VertexBufferLock(vb, offset, size, flags)` → pointer | Common Lock `sub_821C7048` with kind 10, address `[vb+24] & ~3`, size `[vb+28] & 0x03FFFFFC` (`0x821C741C`–`0x821C744C`). **Offset and size are ignored** (r4/r5 overwritten; every caller seen passes 0, 0, 0, e.g. `sub_82414E78` `0x82414E88`–`0x82414E9C`). The pointer is the return value: there is no `data` argument, so the trace's argument names are off by one. POTCAWE `sub_822ACEE0`, 0.77. | 7 in 5 | known |
| `sub_821C64C0` | `VertexBufferUnlock(vb)` | `b CommonUnlock(vb, [vb+24] & ~3, 0)` (`sub_821C6178`). POTCAWE `sub_822ACF30`, 1.00. | 7 in 5 | known |
| `sub_821C7460` | `IndexBufferLock(ib, offset, size, flags)` → pointer | Common Lock kind 12, `[ib+24]`, `[ib+28]`, flags \| 2. POTCAWE `sub_822ACF40`, 0.72. | 8 in 6 | known |
| `sub_821C64D0` | `IndexBufferUnlock(ib)` | `b CommonUnlock(ib, [ib+24], 0)`. POTCAWE `sub_822ACF88`, 1.00. | 8 in 6 | known |
| `sub_821BDA28` | `TextureUnlock(texture)` | `b CommonUnlock(tex, [tex+32] & ~0xFFF, [tex+48] & ~0xFFF)` (fetch dwords 1 and 5). POTCAWE `sub_822AA700`, 1.00. `sub_821BDA40` does the same for an object whose texture is at `+24` (4 sites in `0x821E4xxx`). | 27 in 15 | known |
| `sub_821D00A8` | `FlushCpuRange(start, end)` | `dcbf` over `[start, end)` in 128-byte lines, then `sync`; skips `0x7F100000`–`0x86FFFFFF`. POTCAWE `sub_822AF398`, 0.65. | 0 (16 library: unlocks, constants, the command buffer) | known |
| `sub_821C91C0` | `KickSegment(dev)` → write pointer | Outside a tiling recording (`+13232 == 0`, `+10940 & 0x80` clear) a submission record (`sub_821C85C0`, `sub_821C82F0`, `sub_821C8F40`); then the kick tail `sub_821C9038`: the segment fence `sub_821C8780` (`INVALIDATE_STATE`, two `EVENT_WRITE_SHD`: ring position → `[wb+4]`, fence `[+10908]` → `[wb+0]`, `+10908 += 2`) and the submission, **then continues in the same memory, aligned up to 32 bytes** (`+14928` = new start, `[+48]` = start − 4, `0x821C9178`–`0x821C91B0`), or NewSegment past the threshold (`0x821C918C`–`0x821C919C`). Inside a tiling recording it appends `[0x81000000 | dwords, address]` to the tiling header `+13368` instead (`0x821C9100`–`0x821C9130`). POTCAWE `sub_822AEC50`, 0.82 (but the behavior is GRAW's: continue in place). | 0 (112 library) | known |
| `sub_821C8E08` | `NewSegment(dev, dwords)` | Allocates `(dwords + 14)·4` bytes (or the minimum `+14912`) through `sub_821C80C0`/`sub_821C74A8`/`sub_821C7718`; `+14928` = start, `[+48] = start − 4`, `+52` = end, `+56 = end − 160` (`0x821C8ECC`–`0x821C8EFC`). Callers: the kick tail, the reservation `sub_821C93F8(dev, dwords)` (kick, then NewSegment if it still does not fit), Present (`0x821D1990`). POTCAWE `sub_822AE950`, 0.75. | 0 (3 library) | known |
| `sub_821C82F0` | Command buffer allocator `(dev, dwords, alignment)` | Side allocations: inline draws' vertices/indices, resolve and clear data, replay records. | 0 (13 library) | known |
| `sub_821CC368` | `QueryIssue(query, flags)` | `[q+0]` = device; BEGIN (`flags & 2`): tiles `[q+148]` = `[+12756]` when tiling else 1 (`0x821CC384`–`0x821CC39C`), packets through `sub_821D23E8` (POTCAWE `sub_822C4848`, 0.80: RB_SAMPLE_COUNT_ADDR, bin-predicated per tile, `EVENT_WRITE_ZPD`), or `EVENT_WRITE(25)` for type 10; END (`flags & 1`): the marker `0xFFFFFEED` into `+16/+20` of the last tile's block `[q + 24 + 4·tiles]` — **by the CPU, or by a GPU `MEM_WRITE` + `WAIT_REG_MEM` when the previous result is still pending** (`0x821CC444`–`0x821CC550`); END fence `[+10908]` → `[q+24]`, `[q+20] |= 0x40`. POTCAWE `sub_822BA590`, 0.77. | 10 in 5 | known |
| `sub_821CD008` | `QueryGetData(query, data, size, flags)` | **Pending while the END fence `[q+24]` is not yet reached by the GPU** (`[wb+0]`, `0x821CD048`–`0x821CD0A4`; kicks if it is the current fence), not by the marker; then Σ per tile of `ZPass_A + ZPass_B (+16/+20) − begin (+48/+52)`, byte-reversed, read through the `0xC0000000` view (`0x821CD0C4`–`0x821CD118`). POTCAWE `sub_822BA818`, 0.82. | 5 in 3 | known |
| `sub_821D0F40` | `Swap(dev, frontBuffer)` | SetPredication(0); with no front buffer a `WAIT_REG_MEM` on `[+10900]+4` (`0x821D0F8C`–`0x821D0FEC`); maps the present interval `+13596` (0/1 → 1, 2 → 2, 4 → 3, else 0); descriptor `frontBuffer | interval << 8 | threshold` (`+11860` bits 23–29, `0x821D1030`–`0x821D104C`); GPU callback `sub_821D0D90` through `sub_821C8940` (`0x821D1074`); `+10941 |= 0x10`. **Synchronize** is `sub_821D1410` (`li r4,0; b Swap`). POTCAWE `sub_822B58B0`, 0.68. | 0 direct; 3 through Synchronize | known |
| `sub_821D1418` | `Present(dev, frontBuffer, params)` | `+16560 += 1`; copies the front buffer's fetch constant (`frontBuffer+28`, 24 bytes, `0x821D143C`–`0x821D1458`) and 52 bytes into `+14844`; reserves 64 dwords (`sub_821C93F8`, `0x821D1640`) and **calls `VdSwap`** (`0x821D167C`); compares the params with `+13640`; internal draws through `sub_821DA978` and `sub_821D6680`; the pending-swap wait (`sub_821C5938`/`sub_821C5A48`); `VdPersistDisplay`; NewSegment(`[+14912]`). r4 is the front buffer, as `kPresent` expects. POTCAWE `sub_822B5D58`, 0.76. | 3: `sub_8241C840` (every frame), `sub_822E0B98` (an alternative path of the same), `sub_822E28E8` (init); plus the library's worker `sub_821D2F28` | known |
| `sub_821C3DE0` | Present interval setter | `stw r4,13596(r3)`. | 3, all in the engine's present `sub_8241C840` | known |
| `sub_821C94E8` | `InsertCallback(dev, type, callback, context)` | Kick check, then `sub_821C8940` (`WAIT_REG_MEM` + `INTERRUPT`) at the write pointer. POTCAWE `sub_822AE6C8` matches `sub_821C8940`, 0.51. | 5 in 5 (`sub_8241C840`, `sub_82414F18`, `sub_824143D8`, `sub_8241DA70`, `sub_8241DB00`) | likely |
| `sub_821C7CB8`, `sub_821C7D50`, `sub_821C8870`, `sub_821D1BC8`, `sub_821D1DF8`; `sub_821C5938`, `sub_821C5A48` | GPU wait loops (fence, ring range, ring space, two write-back waits) and the wait start / per-iteration helpers (`db16cyc` ×8, hang check) | Matched with POTCAWE's six loops (0.82–0.95) and helpers (0.95, 0.70); Present's pending-swap wait is the sixth. Not hooked by this game (its CPU fix is in the engine's waits, `src/thread_waits.cpp`). | — | likely |

**The tiling replay worker** `sub_821D2F28` (a thread entry, no callers) waits on an event and, under the device's
critical section (`+14944`), when `+10942 & 0x02`, calls Synchronize and **Present(dev, dev+14844, 0)**
(`0x821D300C`–`0x821D302C`): a re-present on another thread than the render thread, with a copy of the last front
buffer header, as in Kameo.

## 3. The device layout (XDK 2.0.7978)

The device is 24064 bytes (`sub_821C5FE0`, `0x821C5FFC`). The draw's flush, as in DrawIndexedVertices:

```
+0  VS constants  -> sub_821CA358(dev, mask, 0x4000, dev+1920)                    0x821C1E70
+8  PS constants  -> sub_821CA358(dev, mask, 0x4400, dev+6016)                    0x821C1E88
+16 bits 17-20    -> program flush sub_821CB180                                   0x821C1EA8
    & [+40]       -> tile-dependent flush sub_821C9C28(dev, mask, [+10560])       0x821C1EC4
    bits 0-11     -> 0x2200 group, dev+10548 (12)  (register 0x2200 = bit 11)     0x821C1EE8
    bits 12-16    -> 0x2180 group, dev+10528 (5)   (0x2180 = bit 16)              0x821C1F08
    bits 42-57    -> 0x2000 group, dev+10368 (16)  (0x2000 = bit 57)              0x821C1F28
    bits 21-41    -> 0x2100 group, dev+10444 (21)  (0x2100 = bit 41)              0x821C1F54
+24 bits 0-31     -> fetch flush sub_821CA200 (slot s = bit 31 - s), dev+1152    0x821C1F88
    bits 34-54    -> 0x2280 group, dev+10596 (21)  (0x2280 = bit 54)              0x821C1FA0
+32 bits 0-37     -> 0x2300 group, dev+10680 (38)  (0x2300 = bit 37)              0x821C1FD8
    bits 38-45    -> 0x2380 group, dev+10832 (8)   (0x2380 = bit 45)              0x821C1FF8
    bits 49-54    -> misc flush sub_821CA080: coherency sequence, then the user
                     clip planes PA_CL_UCP_0..5 (0x2388 + 4i) from dev+10272     0x821C2020
    bit 56        -> bool/loop constants 0x4900, dev+10112 (40 dwords)            0x821C2040
```

Float constant group g (4 vec4) is bit `63 − g` of its mask (`cntlzd`, 64 bytes per bit, `sub_821CA358`
`0x821CA384`–`0x821CA3A4`). The fetch flush shifts `+24`'s low half up (`rldicr r4,r28,32,31`, `0x821C1F94`) and walks
it with `cntlzd` from `+1152` to register `0x4800 + 6·slot`, then writes three zeros to `0x5000` (`0x821CA2B4`–`0x821CA2D4`,
as GRAW). The flush helpers that run out of room write what fits and kick (`sub_821C9978` → `sub_821C91C0`,
`0x821C9A14`).

| Field | Offset | Evidence (function, instruction) |
|---|---|---|
| Dirty masks (u64): VS constants; PS constants; groups `0x2000`/`0x2100`/`0x2180`/`0x2200` + program; fetch slots (low half) + `0x2280`; `0x2300`/`0x2380` + clip planes + bool/loop; tile-dependent subset of `+16` | `+0`; `+8`; `+16`; `+24`; `+32`; `+40` | The flush above; SetRenderTarget bit 56 of `+16` = RB_COLOR_INFO (`0x821C5358`–`0x821C5380`); SetDepthStencilSurface bit 55 = RB_DEPTH_INFO, bit 8 = RB_HIZCONTROL (`0x821C5778`–`0x821C578C`); inline sampler states set bit 31 of `+24` for slot 0 (`sub_8241D250` `0x8241D604`–`0x8241D60C`) |
| Command buffer write pointer (last dword written, `stwu`) / segment end / kick threshold | `+48` / `+52` / `+56` | NewSegment `0x821C8EDC`, `0x821C8EF8`, `0x821C8EFC`; draws `0x821C2080`/`0x821C2084`/`0x821C2250` |
| User clip planes (6 × 4 floats, `0x2388`–`0x239F`) | `+10272` | Misc flush `sub_821CA080` `0x821CA09C`/`0x821CA0A0` |
| Fetch constants (32 × 24 bytes, `0x4800`) | `+1152` | Fetch flush `0x821CA20C`/`0x821CA21C`; SetTexture; SetStreamSource (vertex fetch 95 − s at `+1912 − 8·s`) |
| VS / PS float constants (256 vec4 each) | `+1920` / `+6016` | Flush `0x821C1E70` / `0x821C1E88`; `sub_821C0550` / `sub_821C0628` |
| Bool (8) + loop (32) constants (`0x4900`) | `+10112` | Flush `0x821C2054` |
| Register shadow `0x2000` (16), `0x2100` (21), `0x2180` (5), `0x2200` (12), `0x2280` (21), `0x2300` (38), `0x2380` (8) | `+10368`, `+10444`, `+10528`, `+10548`, `+10596`, `+10680`, `+10832` | Flush `0x821C1F40`, `0x821C1F68`, `0x821C1F14`, `0x821C1EF4`, `0x821C1FB8`, `0x821C1FE4`, `0x821C200C`; counts from the masks |
| PA_SC_WINDOW_OFFSET / SCISSOR_TL / BR (after the `0x2000` group, not a flushed group) | `+10432` / `+10436` / `+10440` | SetScissorRect `0x821C48F4`–`0x821C4910`; written by `sub_821C2280` |
| Write-back block pointer (`[+0]` last fence reached, `[+4]` ring position) | `+10896` | Segment fence `sub_821C8780` `0x821C8798`, `0x821C884C`/`0x821C8854`; GetData `0x821CD054` |
| Swap/vblank block pointer | `+10900` | Swap `0x821D0FBC` (`WAIT_REG_MEM` on `[+10900]+4`) |
| Fence counter (+2 per segment; stamped at `[resource+8]` on unbind) | `+10908` | `sub_821C8780` `0x821C87A8`, `0x821C885C`; SetIndices `0x821C4B70`/`0x821C4B7C` |
| Mask for deferred fencing (when the counter is 0, resources go to a list at `+13524/+13528`) | `+10912` | SetIndices `0x821C4B84`–`0x821C4BD0` |
| Current GPR allocation | `+10920` | SetShaderGPRAllocation `0x821C0D3C` |
| Predication copies | `+10932`, `+10936` | SetPredication `0x821D039C`/`0x821D03A4` |
| Flag bytes | `+10940`: `0x01` tiling targets bound (draws record extents), `0x20` tiling, `0x04`/`0x08`/`0x10` (other cases of the "bound" test), `0x80` (a special mode: the kick skips submission, literals inlined); `+10941`: `0x10` swap pending, `0x20` (allocation failed), `0x02`; `+10942`: `0x80` (cleared by SetVertexShader), `0x20` HiZ from the depth surface, `0x02` worker re-presents; `+10943`: `0x08` (RT0 format), `0x20` BeginTiling skips its clear, `0x80` extents query open | Draws `0x821C2158`; BeginTiling `0x821D060C`/`0x821D04C8`; EndTiling `0x821D0AA8`/`0x821D0A9C`; kick `0x821C91F4`; Swap `0x821D10F8`; NewSegment `0x821C8E1C`; SetVertexShader `0x821C0A3C`; SetDepthStencilSurface `0x821C56B0`; worker `0x821D300C`; SetRenderTarget `0x821C5474`; QueryIssue `0x821CC3F8` |
| Current vertex declaration | `+11824` | SetVertexDeclaration `0x821C0B68`; program flush `0x821CB1D0` |
| Stream strides the VS was patched for (bytes / 4) | `+11840 + s` | SetStreamSource `0x821C4AA8`; BeginVertices `0x821C0FA4` |
| Packed render-state word (PRESENTIMMEDIATETHRESHOLD in bits 23–29) | `+11860` | Swap `0x821D1030`/`0x821D103C`; setters `sub_821C3DF0`, `sub_821C2608`… |
| Scissor test enable | `+11864` | `sub_821C51C8`; SetScissorRect `0x821C48A0` |
| Per-RT color write masks (RT0–3) | `+11868 + 4·i` | SetRenderTarget `0x821C5584`, `0x821C55C0`, `0x821C55EC`, `0x821C5618` |
| Per-sampler bytes | `+11958 + i`, `+11984 + i` | SetTexture `0x821BFAA4`, `0x821BFAE8` |
| Visibility-select bytes for the draw packet | `+12130[+12195]` | Draws `0x821C2074`, `0x821C20DC` |
| Index buffer | `+12444` | SetIndices `0x821C4BD4`; DrawIndexedVertices `0x821C2070` |
| Render targets RT0–RT3 | `+12448…+12460` | SetRenderTarget `0x821C5314`–`0x821C5324` |
| Depth-stencil surface | `+12464` | SetDepthStencilSurface `0x821C5670` |
| Vertex buffers, one pointer per stream | `+12468 + 4·s` | SetStreamSource `0x821C4A18`, `0x821C4A94` |
| Stream strides (bytes / 4) | `+12536 + s` | SetStreamSource `0x821C4AA0`; BeginVertices `0x821C0FB4` |
| Textures (26 samplers) | `+12552 + 4·i` | SetTexture `0x821BFA1C`, `0x821BFB14` |
| Viewport: X, Y, W, H, MinZ, MaxZ (floats); `+12680` = 0 | `+12656…+12676` | `sub_821C4F70` `0x821C50F0`–`0x821C5138`; SetScissorRect reads them (`0x821C484C`–`0x821C4874`) |
| Scissor rect (x1, y1, x2, y2) | `+12684…+12696` | SetScissorRect `0x821C48A4`–`0x821C48B8` |
| Pixel shader; vertex shader | `+12700`; `+12704` | SetPixelShader `0x821C085C`; SetVertexShader `0x821C0A38`; program flush `0x821CB1CC`/`0x821CB1D4` |
| Predication mask | `+12716` | SetPredication `0x821D0370`/`0x821D0380` |
| Predication active (mask ≠ 0); `+12720` (0) | `+12724` | SetPredication `0x821D0388`, `0x821D0294`; SetShaderGPRAllocation `0x821C0D40` |
| Set to 2 by BeginTiling | `+12728` | BeginTiling `0x821D0634` |
| Reference surface (RT0 else depth), saved RT0–RT3, depth | `+12732`; `+12736…+12748`; `+12752` | BeginTiling `0x821D061C`–`0x821D0630`; EndTiling `0x821D07D0`, `0x821D0AAC`; every "tiling targets bound" test |
| Tile count | `+12756` | BeginTiling `0x821D03FC`; EndTiling `0x821D07A4`; QueryIssue `0x821CC390` |
| Tile rects (x1, y1, x2, y2) | `+12760 + 16·i` | BeginTiling `0x821D0430`–`0x821D0470`; EndTiling default `addi r5,r3,12760` (`0x821D0798`) |
| 32-aligned tile corners (x1, y1) | `+13000 + 8·i` | BeginTiling `0x821D0424`–`0x821D0488` |
| Tiled area (W, H); BeginTiling flags | `+13188`, `+13192`; `+13196` | BeginTiling `0x821D04B0`–`0x821D04BC`; `sub_821C4F70` `0x821C5060`; ClearF `0x821DBDEC` |
| Tiling clear color (float4) / Z / stencil | `+13200` / `+13216` / `+13220` | BeginTiling `0x821D05C8`–`0x821D05E8`; EndTiling `0x821D0884`–`0x821D0890` |
| Tiling recording active | `+13232` | Kick `0x821C91E8`; kick tail `0x821C90D4` |
| Extents record buffer (descriptor; current `+13256`, limit `+13260`) | `+13248` | Draws `0x821C2188`–`0x821C21BC` |
| Tiling header buffer (descriptor; current `+13376`, limit `+13380`) | `+13368` | BeginTiling `0x821D06B0`–`0x821D06E0`; kick tail `0x821C9100`–`0x821C9130`; EndTiling `0x821D0948` |
| Begin*Vertices: end of the packets; vertex / index data; vertex / index dword counts | `+13444`; `+13448` / `+13452`; `+13456` / `+13460` | BeginVertices `0x821C1418`–`0x821C1420`; BeginIndexedVertices `0x821C1960`–`0x821C1970`; EndVertices; DrawVerticesUP `0x821C1478` |
| Deferred-fence list (current, limit) | `+13524`, `+13528` | SetIndices `0x821C4B94`–`0x821C4BD0` |
| D3DRS_PRESENTINTERVAL | `+13596` | `sub_821C3DE0`; Swap `0x821D0FF0` |
| Last scaler parameters | `+13640` | Present `0x821D1580` |
| Prebuilt GPR buffers (address, size pairs) | `+13784…`, `+14816/+14820` | SetShaderGPRAllocation `0x821C0D7C`–`0x821C0DBC` |
| Copy of the last front buffer header (52 bytes) | `+14844` | Present `0x821D1498`–`0x821D14B8`; worker `0x821D3024` |
| Segment size used by Present's NewSegment and as a minimum (*likely*); segment start | `+14912`; `+14928` | NewSegment `0x821C8E64`, `0x821C8ED4`; kick tail `0x821C9050`, `0x821C91AC` |
| Device critical section | `+14944` | Worker `0x821D3000` |
| Present count | `+16560` | Present `0x821D1434`–`0x821D1448` |

Also in the shadow and worth knowing: RB_SURFACE_INFO `+10368` (`0x2000`), RB_COLOR_INFO `+10372`, RB_DEPTH_INFO
`+10376`, RB_COLOR_MASK `+10460` (`0x2104`), PA_CL_VPORT_* `+10504…+10524`, SQ_PROGRAM_CNTL/CONTEXT_MISC/
INTERPOLATOR_CNTL `+10528/+10532/+10536`, RB_DEPTHCONTROL `+10548`, RB_HIZCONTROL `+10560`, PA_SC_AA_CONFIG
`+10684`, RB_COPY_CONTROL…DEST_INFO `+10776…+10788`.

## 4. Resource objects

Identical to POTCAWE's (XDK 5426), each offset read here.

| Object | Field | Offset | Evidence |
|---|---|---|---|
| Any resource | Common word (type, lock/bind counts; bit 31 = 32-bit indices for an index buffer) | `+0` | DrawIndexedVertices `0x821C20E8`/`0x821C2114` |
| Any resource | Fence of its last use (stamped on unbind) | `+8` | SetIndices `0x821C4B7C`; SetTexture `0x821BFB2C`; SetStreamSource `0x821C4A38` |
| Texture | Fetch constant, 6 dwords (`+28` dword 0 … `+48` dword 5; size in dword 2 at `+36`) | `+28` | SetTexture `0x821BFA34`–`0x821BFA54`; Resolve `0x821DC6F0`–`0x821DC744`; Present copies `frontBuffer+28` (`0x821D143C`); TextureUnlock reads `+32`/`+48` |
| Surface | RB_SURFACE_INFO; RB_COLOR_INFO / RB_DEPTH_INFO; RB_HIZCONTROL; EDRAM bytes (tiles × 5120) | `+24`; `+28`; `+32`; `+44` | `sub_821C5278` `0x821C5278`/`0x821C52AC`; SetRenderTarget `0x821C5340`; SetDepthStencilSurface `0x821C5690`, `0x821C56A4`; XGSetSurfaceHeader `0x821BF328`, `0x821BF348`/`0x821BF3EC`, `0x821BF388`, `0x821BF3F4` |
| Surface | Size: width − 1 in bits 18–31, height − 1 in bits 3–17 | `+36` | XGSetSurfaceHeader `0x821BF2F4`–`0x821BF320`; `sub_821C4F70` `0x821C506C`–`0x821C507C` (`rlwinm 14,18,31` / `rlwinm 29,17,31`); ClearF `0x821DBD1C`–`0x821DBD30`; EndTiling `0x821D07E8`–`0x821D0810` |
| Vertex / index buffer | Base address (a vertex buffer's is its fetch dword 0, type bits included); size (fetch dword 1) | `+24`; `+28` | VB Lock `0x821C741C`/`0x821C7424`; IB Lock `0x821C746C`/`0x821C7478`; SetStreamSource `0x821C49CC`/`0x821C49D4`; DrawIndexedVertices `0x821C20D4` |
| Query | Device; type (9 occlusion, 10 extents, 8); state (2 begun, 1 ended); flags byte; END fence; block pointers per tile; tile count | `+0`; `+4`; `+16`; `+20`; `+24`; `+28 + 4·t`; `+148` | QueryIssue `0x821CC374`, `0x821CC3A0`, `0x821CC408`/`0x821CC5D0`, `0x821CC448`, `0x821CC5D4`–`0x821CC5E4`, `0x821CC42C`–`0x821CC440`, `0x821CC39C`; GetData `0x821CD048`, `0x821CD0C4`–`0x821CD0D0` |
| Vertex shader | Literal block `vs + 872 + [vs+892]` (Dante's `+892`; POTCAWE `+896`): u64 constant-group mask, u64 bool flag, records size `+16`, records `{u16 offset, u16 count}` from `+20` | — | SetVertexShader `0x821C0A48`–`0x821C0A98` |
| Pixel shader | Literal block `ps + 40 + [ps+60]`, same format | — | SetPixelShader `0x821C087C`–`0x821C08CC` |

## 5. Engine behavior

| Topic | What the engine does |
|---|---|
| Inline device writes | A scan of the engine for writes through the device pointer (`[0x82AA0C80 + 2292]`): **VS constants** written inline in 18 functions and **PS constants** in 33 (e.g. `sub_821B4FB0` `stfs f11,1932(r9)` `0x821B5844`, then `ld/or/std 0(r11)`); the masks `+0` in 22, `+8` in 48, `+24` in 34; **fetch constants** in 30, all inlined sampler states that patch a slot's dword 0 and set its dirty bit (e.g. `sub_8241D250` `0x8241D5F8`–`0x8241D60C`, clamp bits of slot 0); vector stores with computed offsets in 23. **No** inline write to the write pointer, the register shadow, the current objects (targets, textures, buffers, declaration) or the shader fields (only the library writes `+12700/+12704`), and no PM4 written by the engine. |
| Setter arguments | SetTexture, SetStreamSource and the constant setters take the dirty bit or mask from their caller (r6, r8, r7): the engine computes it inline (`srd r6,r26,r10` with `r26 = 1 << 63` and `r10 = sampler + 32`, i.e. bit
`31 − sampler`, in `sub_824A8940` `0x824A8988`/`0x824A8DAC`–`0x824A8DBC`; `srad/srd` for constants, `sub_821B4FB0` `0x821B5874`–`0x821B5878`). |
| BeginVertices / EndVertices | The engine calls BeginVertices and EndVertices itself in one function, `sub_821B2ED8` (reached through a function pointer: its caller `sub_821B43D0` has no direct caller), with no D3D call between. DrawVerticesUP (31 sites) and DrawIndexedVerticesUP (4 sites) inline the end. There is no inlined end anywhere else (`ppc_index.py grep 13444`: only the library; `sub_821BBE58`'s `13444` is a global, `0x82823484`). |
| Draw submitters | Meshes: `sub_824145E8`, `sub_824146E0`, `sub_824147E8` (SetVertexDeclaration cached at `0x82AA0C80+12880`, SetStreamSource ×1–2, SetIndices, DrawIndexedVertices(prim `[obj+132]`, 0, 0, `lhz [obj+92]`)); `sub_82414F18` (two DrawIndexedVertices with queries and a GPU callback); `sub_8247B998` (DrawIndexedVertices + 3 DrawVerticesUP). DrawVertices: `sub_82447D08` (with a query), `sub_8244A9D0` ×3, `sub_8244AB80` ×2, `sub_824CE7E8`, `sub_824CF8E0`, `sub_824CF980` ×2, `sub_824CFA68`. DrawIndexedVerticesUP: `sub_8244C968` ×3, `sub_824D5AA0`. |
| Shaders and textures | The material binder `sub_824A8940` (4 callers) calls SetTexture per sampler, then SetVertexShader / SetPixelShader only when they change (cache at `0x82AA0C80+12872/+12876`). |
| Targets | Through the alias `sub_821C5930` (75 sites) and SetDepthStencilSurface (52). |
| Tiling | `sub_82443570` → `sub_8243D298`: SetRenderTarget/SetDepthStencilSurface from the renderer's per-mode config (`cfg = 0x82AA0C80 + 224·[+4136]`), then BeginTiling(dev, 0, `[cfg+3252]`, `cfg+3256`, NULL, Z from `0x82000A94`, 0); the end `sub_8243D3D8`: per tile a **depth Resolve (flags `0x14`) into `[[obj+332]+168]`** at `cfg+3384 + 8·t`, without SetPredication, then **EndTiling(dev, 0x300, NULL, `[[obj+332]+88]`, NULL, Z, 0, 0)**. Both take the tiled path only when `[[[obj+116]+332]+16] & 0x10000000` and `[cfg+2356] > 1` (`0x8243D40C`–`0x8243D444`); otherwise `0x8243DA60`. |
| Present | The render thread's `sub_8241C840` (tail-called from `sub_822DB7C0`): Synchronize (`0x8241C900`), `Present(dev, frontBuffer, 0x82AA0C80+2296)` (`0x8241C910`) with two alternating front buffers at `0x82AA0C80+2336/+2340`; the other branch inserts a callback and calls `sub_822E0B98` (Synchronize + Present). It also sets the present interval (`sub_821C3DE0` ×3). |
| Queries | Issue from `sub_82414F18`, `sub_8243F210`, `sub_82440058`, `sub_82442A30`, `sub_82447D08`; GetData from `sub_82414F18`, `sub_8243E9A0` ×3, `sub_82524CF8`. |
| GPR allocation | `sub_823A3DD8`, `sub_823A4588`. |
| Output size | The presentation parameters come from a video-mode table (`sub_822E2098` picks an entry of `0x82AA0C18` by the display mode, `XGetVideoMode`); not a constant in the image. |

## 6. Differences from POTCAWE (5426) and Dante's Inferno

- **Offsets.** Masks, command buffer fields (`+48/+52/+56`) and their arrangement are POTCAWE's. Constants, bool/loop,
  the register shadow, the write-back block, swap block, fence counter and flags are POTCAWE's + 128, which is
  exactly Dante's Inferno's column (Dante's fence `+10908` and shadow `+10368…` are the same). The current objects
  are POTCAWE's + 144 = Dante's − 168 (vertex declaration POTCAWE + 140); BeginVertices' saved end is POTCAWE + 144
  (Dante − 208); the present interval POTCAWE + 152. The resource objects are POTCAWE's, and the vertex shader's
  literal block offset is Dante's (`+892`).
- **The draws are inside the library**, not apart (POTCAWE `0x824ABxxx`).
- **New entry points**: BeginIndexedVertices, EndVertices (as functions, GRAW had them), DrawIndexedVerticesUP.
- **Setters take the dirty bit from the caller** (SetTexture r6, SetStreamSource r8, the constant setters r7).
- **The kick continues in the same memory** (aligned to 32 bytes) and only calls NewSegment past the threshold:
  GRAW's behavior, not POTCAWE's.
- **Tiling uses `SET_BIN_MASK_LO`/`SET_BIN_SELECT_LO`/`_HI`** (`0x60`–`0x63`) instead of `SET_BIN_MASK`/`SELECT`.
- **EndTiling is given a destination** by this engine and resolves each tile itself; BeginZPass/EndZPass are not
  linked.
- **Queries**: GetData's "pending" is the END fence against the write-back fence, not the marker; the END marker
  can be written by the GPU (`MEM_WRITE`) when the previous result is still pending.
- **SQ_PROGRAM_CNTL/CONTEXT_MISC/INTERPOLATOR_CNTL are in the shadow** (and SQ_PROGRAM_CNTL/CONTEXT_MISC also written
  straight into the command buffer by the program flush); shader literals still use `LOAD_ALU_CONSTANT`, as in
  POTCAWE.
- **VertexBufferLock/IndexBufferLock return the pointer** and ignore offset and size.

## 7. Verification

**Trace** (`--native_x_renderer_trace=true`, 2026-10-24, the intro videos and the title screen, which the game
reaches by itself): every call on one render thread; per frame 7 DrawVerticesUP (each through BeginVertices), 11
SetRenderTarget, 10 SetDepthStencilSurface, 3-4 Clear, 8 Resolve, about 79 SetTexture, 69 VS and 73 PS constant
setters, 1 KickSegment (3 more inside the library) with 3 NewSegment, 2 SetShaderGPRAllocation, 1 Swap, 1 Present.
The dumped frame (2400) has 342 calls.

**Observer** (`--native_x_renderer_verify_at=2700`, the title screen): 7 of 7 draw calls fully match the rebuilt
state (100 %); the 13 other draw packets are the library's own (Clear ×5, Resolve ×8); no render state, vertex
fetch, texture fetch, constant, program, bool or loop mismatch; no packet from outside the ring; no memexport.

**Observer in the main menu** (F10, frame 3645): 189 of 189 draw calls (89 DrawIndexedVertices, 51 DrawVertices, 49
DrawVerticesUP) fully match; 37 packets of the library's own Clears and Resolves; no mismatch of any kind.

**Observer in a song** (Quickplay, Ace of Spades, F10, frame 11593): 654 of 654 draw calls (502 DrawIndexedVertices, 7
DrawVertices, 144 DrawVerticesUP and the engine's inline BeginVertices…EndVertices draw, found through
`inline_draw_end`) fully match; 15,496 VS and 5,974 PS constants compared, no mismatch; 65 packets of the library's
own Clears and Resolves; one register group per tile (the game does not tile at 720p: no BeginTiling in the trace),
no memexport. Per frame in a song: about 264 DrawIndexedVertices, 136 DrawVerticesUP, 27 Resolves, 55 kicks.

**First frames** (`--native_x_renderer=true`, 2026-10-24): the loading screen, the intro, the title and the menus look
as on the Xenos path. In songs the 3D scene was covered with noise and vertical stripes. A RenderDoc capture of
NativeXRenderer's device showed the post-processing reading a 640×360 resolve whose host target nobody drew: the
engine draws the 1280×720 scene into a 1x surface at EDRAM base 0, then resolves a 640×360 4x MSAA surface at the
same base with no draw in between, a 2×2 average of the scene. NativeXRenderer's D95 resolves it from the 1x target;
the career's opening cinematic and its first song in the arena are then clean (`ghmetallica_059.log`: "a resolve
through a 640x360 surface with 4x MSAA reads the 1280x720 target drawn at the same EDRAM").

**Antialiasing:** the copy of the finished scene before the HUD is the profile's `scene_resolve_caller`, `0x8251F04C`
(called from `0x824B80C0`): after the post-processing, the engine resolves the 1280×720 scene into `0x40008BE0` and
its last full-screen pass (the bloom composite) draws it back, then the HUD and the note highway go over it (the
RenderDoc capture: the composite reads that texture). It comes once a frame, in the menus too (their 3D background,
before the 2D layer). With `native_x_renderer_antialiasing = smaa` the log reports SMAA applied in 560-596 of every
600 frames; the menus' text and the HUD stay sharp.

**Ultrawide:** `src/widescreen.cpp` takes NativeXRenderer's `AspectFactor()` when it draws (`widens_cameras` in the
profile), so with `native_x_renderer_fill_screen` the stage is seen wider and the 2D layer stays 16:9 in the center,
as on the Xenos path's anamorphic ultrawide (3440×1440: the same image on both paths).

**Performance A/B** (2026-10-24, `local_docs/04`, the career's first gig: the title, the menu, the opening
cinematic 25 s in, the song 65 s and 85 s in; the same anisotropic filtering, no antialiasing, the Xenos path at
the output's resolution scale): both at 60 FPS everywhere, the longest frame 17.6-20.1 ms on both. GPU use and power
in the song:

| Output | Xenos | NativeXRenderer |
|---|---|---|
| 1280×720 (window) | 29 % / 51.6 W | 22 % / 45.6 W |
| 2560×1440 | 45 % / 96 W | 27 % / 53.3 W |
| 3440×1440 (ultrawide) | 59 % / 123.5 W | 31 % / 55.9 W |

The idle desktop draws about 40 W; at 3440×1440 the Xenos path also raised the GPU clock (2865 MHz against 2745).
CPU: 1.2-1.3 cores on the Xenos path, 1.3-1.45 with NativeXRenderer (its renderer thread); the busiest thread, about
43 % of a core in songs and 73 % in the cinematic, is the game's own on both. Images: the menus match within 0.3 of 255
on average; the cinematic and the songs differ by 7-16, the camera at a slightly different moment.

Seen but not handled: the crowd's impostor atlas (768×1024) gets its mips 1-5 by resolves to those levels, which
NativeXRenderer skips (it resolves level 0 only); its host texture has one level, so the crowd is sampled at level 0
(sharper, possibly shimmering at a distance).

**The post-processing mask** (2026-10-24, Gustavo's play, NativeXRenderer D96).
- **The mask:** a 128×64 k_8 texture at `0x76ED000` (texture object `0x40009110`).
  - The engine resolves it once, when the career's first song loads, and never again.
  - It binds it to sampler 5 (`0x82519114`, from `0x8251EFB4`, just before the scene's copy for the antialiasing)
    for the post-processing's full-screen composite, which blends the sharp and the blurred scene through it.
  - That composite samples it only in the pause menu, on the failed-song screen and in some moments of a song.
- **The symptom:** NativeXRenderer freed resolved textures after a minute unused (D52). After a minute of playing
  without those moments, the mask was gone and the composite read guest memory, which no resolve had written: the
  stage under a blocky pattern.
- **The fix:** D96 keeps the resolved textures that still hold their resolve, up to 384 MB
  (`native_x_renderer_resolved_memory_mb`). This game's 22 take 117 MB at 3440×1440.

## 8. Open questions

1. **Inline draws.** The profile relies on D77 (`inline_draw_end = 13444`) for both DrawIndexedVerticesUP (no
   `calls.inc` id) and the engine's BeginVertices…EndVertices pair (`sub_821B2ED8`), with EndVertices **not**
   hooked. Hooking EndVertices as well would make D77's "not committed before the next device call" warning fire
   (the hooked EndVertices is that next call) and drop the begin's state. Check in the trace that both paths occur,
   how often, and that the observer sees no draw packet outside a hooked call. The alternative is a library change:
   a `DrawIndexedVerticesUP` call id handled like DrawVerticesUP, then hook EndVertices and leave
   `inline_draw_end` unset.
2. **EndTiling with a destination** (not reached at 720p: the game does not tile there) (flags `0x300`, `[[obj+332]+88]`): NativeXRenderer's renderer logs it as "not
   supported yet" (D60, `ExecuteEndTiling`). The observer is not affected (EndTiling's nested Resolve and replay
   packets are part of its own call). For step 4 it needs a library change (per tile: resolve the tile's rect into
   the destination at its corner, then the clears `0x100`/`0x200` with the tiling clear values). Whether it matters
   depends on question 3.
3. **Does the game tile?** Only when `[cfg+2356] > 1` (probably the MSAA level of the video mode) and a flag in the
   scene object. The trace will show BeginTiling's count and rectangles, and the engine's per-tile depth resolves
   (`0x14`, inside the bracket, without SetPredication): how the library confines each to its tile.
4. **Output size**: 1280×720 assumed; the presentation parameters come from a video-mode table at run time. The
   trace's Present (front buffer fetch constant) will tell, at 720p and at another console video mode if the SDK can
   report one.
5. **`scene_resolve_caller`** (settled: `0x8251F04C`, section 7): needs the trace (the finished scene before the HUD). If the scene's color is resolved
   by EndTiling, that resolve is nested and the field cannot name it: another reason for question 2.
6. **Queries and D32**: Issue END can make the GPU write the `0xFFFFFEED` marker (`MEM_WRITE`) and wait for it when
   the previous result is pending, and GetData decides "pending" by the fence. Check that NativeXRenderer's host
   count (D32: written into the last tile's end struct) is not overwritten by that marker, and that GetData returns
   it.
7. **Threads**: Present can run on the library's worker thread (`sub_821D2F28`, `+10942 & 0x02`) with
   `dev+14844` as the front buffer; check the trace's thread summary, and which threads lock and unlock buffers.
8. **User clip planes**: the misc flush uploads `PA_CL_UCP_0..5` from `+10272`; the profile's free eighth register
   group could cover them. Add it only if the observer reports differences there.
9. **Trace argument names**: VertexBufferLock/IndexBufferLock are `(buffer, offset, size, flags)` → pointer here, so
   the trace's `data` column is the flags. SetStreamSource, SetTexture and the constant setters have one more
   argument (the dirty bit or mask) than `calls.inc` names.
10. **`widens_cameras`** (settled, section 7): the ultrawide hooks widen by `native_x_renderer::AspectFactor()` when
    NativeXRenderer draws.
11. Not checked statically (should not matter): `sub_821C5E28` (a library function that calls SetRenderTarget,
    SetVertexShader and SetPixelShader: a state restore, *likely*), the meaning of `+12728 = 2`, `+10912`, the bits of
    `+10941`–`+10943` beyond those listed, Present's internal draws (`sub_821DA978`, `sub_821D6680`), and the replay
    builders (`sub_821D30C0`, `sub_821D21F0`, `sub_821D5030`).
