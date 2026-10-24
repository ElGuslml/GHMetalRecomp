// ghmetallica - NativeXRenderer for Guitar Hero: Metallica: the game's D3D entry points and its D3D device and
// object layouts.
//
// The XDK Direct3D library is linked into default.xex (XDK 2.0.7978) at 0x821BF408-0x821DD6F0, the draws included
// (0x821C0F98-0x821C2278), with XGraphics-style helpers below it (0x821BD8E0-0x821BF400). The device layout is
// Dante's Inferno's up to the register shadow (POTCAWE's + 128), the current objects POTCAWE's + 144; the resource
// objects are POTCAWE's. Every address and offset here, with its evidence, is in
// docs/native-x-renderer-ghmetallica.md. The hooks only run the original function unless a NativeXRenderer feature
// is on (--native_x_renderer_trace, --native_x_renderer_verify, --native_x_renderer).

#include "native_x_renderer_ghmetallica.h"

#include <native_x_renderer/hooks.h>

// Draws. DrawVerticesUP and DrawIndexedVerticesUP (sub_821C19A0, 4 engine sites, no call of its own) commit their
// packets with an inlined EndVertices ([dev+48] = [dev+13444]); the engine's only BeginVertices / EndVertices pair
// (sub_821B2ED8) calls EndVertices (sub_821C1430), which is that same store. EndVertices is not hooked:
// inline_draw_end commits all of them.
NATIVE_X_RENDERER_HOOK(821C1E30, DrawIndexedVertices)
NATIVE_X_RENDERER_HOOK(821C1A40, DrawVertices)
NATIVE_X_RENDERER_HOOK(821C1440, DrawVerticesUP)  // calls BeginVertices, inlines the end
NATIVE_X_RENDERER_HOOK(821C0F98, BeginVertices)
NATIVE_X_RENDERER_HOOK(821C1488, BeginIndexedVertices)  // only from DrawIndexedVerticesUP

// Render targets, clears, resolves, tiling. The engine sets targets through sub_821C5930, an alias that branches to
// SetRenderTarget. EndTiling resolves the tiles itself (the engine passes flags 0x300 and a destination).
NATIVE_X_RENDERER_HOOK(821C5308, SetRenderTarget)
NATIVE_X_RENDERER_HOOK(821C5658, SetDepthStencilSurface)
NATIVE_X_RENDERER_HOOK(821DBE18, Clear)  // calls ClearF
NATIVE_X_RENDERER_HOOK(821DBCF0, ClearF)
NATIVE_X_RENDERER_HOOK(821DC640, Resolve)
NATIVE_X_RENDERER_HOOK(821D03E0, BeginTiling)
NATIVE_X_RENDERER_HOOK(821D0750, EndTiling)
NATIVE_X_RENDERER_HOOK(821D0270, SetPredication)
NATIVE_X_RENDERER_HOOK(821C51D8, SetViewport)  // D3DVIEWPORT9 (integers), stored as floats at +12656
NATIVE_X_RENDERER_HOOK(821C4838, SetScissorRect)

// Shaders, vertex input, textures, constants. The engine writes constants straight into the device too (VS in 18
// functions, PS in 33) and patches sampler states in the fetch constants, without a call. SetTexture, SetStreamSource
// and the constant setters take the dirty bit or mask from their caller.
NATIVE_X_RENDERER_HOOK(821C0998, SetVertexShader)
NATIVE_X_RENDERER_HOOK(821C07D8, SetPixelShader)
NATIVE_X_RENDERER_HOOK(821C0B68, SetVertexDeclaration)
NATIVE_X_RENDERER_HOOK(821C49A8, SetStreamSource)
NATIVE_X_RENDERER_HOOK(821C4B50, SetIndices)
NATIVE_X_RENDERER_HOOK(821BFA10, SetTexture)
NATIVE_X_RENDERER_HOOK(821C0550, SetVertexShaderConstantF)
NATIVE_X_RENDERER_HOOK(821C0628, SetPixelShaderConstantF)

// Resources: typed thunks over the common Lock (sub_821C7048) and Unlock (sub_821C6178), and the cache flush over a
// range. The locks return the pointer (no data argument) and ignore the offset and size.
NATIVE_X_RENDERER_HOOK(821C7410, VertexBufferLock)
NATIVE_X_RENDERER_HOOK(821C64C0, VertexBufferUnlock)
NATIVE_X_RENDERER_HOOK(821C7460, IndexBufferLock)
NATIVE_X_RENDERER_HOOK(821C64D0, IndexBufferUnlock)
NATIVE_X_RENDERER_HOOK(821BDA28, TextureUnlock)
NATIVE_X_RENDERER_HOOK(821D00A8, FlushCpuRange)

// Command buffer, queries, frame. The kick submits the segment and continues in the same memory; NewSegment moves the
// write pointer to a newly allocated segment (from the kick, the reservation sub_821C93F8 and Present). The engine
// calls Synchronize (sub_821D1410, Swap with no front buffer), then Present, which calls VdSwap with the front buffer
// in r4; the library's worker thread sub_821D2F28 can present again.
NATIVE_X_RENDERER_HOOK(821C91C0, KickSegment)
NATIVE_X_RENDERER_HOOK(821C8E08, NewSegment)
NATIVE_X_RENDERER_HOOK(821C0CF0, SetShaderGPRAllocation)
NATIVE_X_RENDERER_HOOK(821CC368, QueryIssue)
NATIVE_X_RENDERER_HOOK(821CD008, QueryGetData)
NATIVE_X_RENDERER_HOOK(821D0F40, Swap)
NATIVE_X_RENDERER_HOOK(821D1418, Present)

namespace ghmetallica {

const native_x_renderer::Profile& NativeXRendererProfile() {
  static const native_x_renderer::Profile profile = [] {
    native_x_renderer::Profile p;
    p.game = "Guitar Hero: Metallica (41560830)";
    p.xdk_version = "2.0.7978";
    p.output_width = 1280;  // the presentation parameters come from a video-mode table at run time
    p.output_height = 720;
    native_x_renderer::DeviceLayout& d = p.device;
    // Six dirty masks from +0: VS constants, PS constants, register groups and the program, fetch slots (low half,
    // slot 0 in bit 31) and the 0x2280 group, the 0x2300/0x2380 groups with the clip planes and bool/loop constants,
    // the tile-dependent state.
    d.vs_constants_dirty = 0;
    d.ps_constants_dirty = 8;
    d.fetch_constants_dirty = 24;
    d.fetch_dirty_slot0_bit = 31;
    d.command_buffer_write = 48;
    d.fetch_constants = 1152;
    d.vs_float_constants = 1920;
    d.ps_float_constants = 6016;
    d.bool_loop_constants = 10112;
    d.register_groups = {{
        {10368, 0x2000, 16},
        {10444, 0x2100, 21},
        {10528, 0x2180, 5},
        {10548, 0x2200, 12},
        {10596, 0x2280, 21},
        {10680, 0x2300, 38},
        {10832, 0x2380, 8},
    }};
    d.fence_counter = 10908;
    d.write_back_block = 10896;
    d.flags = 10940;
    d.vertex_declaration = 11824;
    d.index_buffer = 12444;
    d.render_targets = 12448;
    d.depth_stencil = 12464;
    d.vertex_buffers = 12468;  // one pointer per stream
    d.textures = 12552;
    d.viewport = 12656;  // x, y, width, height, min z, max z (floats)
    d.pixel_shader = 12700;
    d.vertex_shader = 12704;
    d.predication_mask = 12716;
    d.tile_count = 12756;
    d.tile_rects = 12760;
    // DrawVerticesUP, DrawIndexedVerticesUP and EndVertices commit BeginVertices' / BeginIndexedVertices' packets
    // with [dev+48] = [dev+13444].
    d.inline_draw_end = 13444;
    native_x_renderer::ObjectLayout& o = p.objects;
    o.texture_fetch = 28;
    o.surface_info = 24;
    o.surface_color_depth_info = 28;
    o.surface_size = 36;
    o.surface_width_minus_one = {18, 14};
    o.surface_height_minus_one = {3, 15};
    o.buffer_address = 24;
    o.buffer_size = 28;
    return p;
  }();
  return profile;
}

}  // namespace ghmetallica
