/* Direct3D Vertex Buffer
 * Copyright (c) 2002 Lionel ULMER
 * Copyright (c) 2006 Stefan DÖSINGER
 *
 * This file contains the implementation of Direct3DVertexBuffer COM object
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#include "ddraw_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(ddraw);

static inline IDirect3DVertexBufferImpl *impl_from_IDirect3DVertexBuffer(IDirect3DVertexBuffer *iface)
{
    return CONTAINING_RECORD(iface, IDirect3DVertexBufferImpl, IDirect3DVertexBuffer_iface);
}

static inline IDirect3DVertexBufferImpl *impl_from_IDirect3DVertexBuffer7(IDirect3DVertexBuffer7 *iface)
{
    return CONTAINING_RECORD(iface, IDirect3DVertexBufferImpl, IDirect3DVertexBuffer7_iface);
}

/*****************************************************************************
 * IUnknown Methods
 *****************************************************************************/

/*****************************************************************************
 * IDirect3DVertexBuffer7::QueryInterface
 *
 * The QueryInterface Method for Vertex Buffers
 * For a link to QueryInterface rules, see IDirectDraw7::QueryInterface
 *
 * Params
 *  riid: Queried Interface id
 *  obj: Address to return the interface pointer
 *
 * Returns:
 *  S_OK on success
 *  E_NOINTERFACE if the interface wasn't found
 *
 *****************************************************************************/
static HRESULT WINAPI IDirect3DVertexBufferImpl_QueryInterface(IDirect3DVertexBuffer7 *iface,
        REFIID riid, void  **obj)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);

    TRACE("iface %p, riid %s, object %p.\n", iface, debugstr_guid(riid), obj);

    /* By default, set the object pointer to NULL */
    *obj = NULL;

    if ( IsEqualGUID( &IID_IUnknown,  riid ) )
    {
        IUnknown_AddRef(iface);
        *obj = iface;
        TRACE("  Creating IUnknown interface at %p.\n", *obj);
        return S_OK;
    }
    if ( IsEqualGUID( &IID_IDirect3DVertexBuffer, riid ) )
    {
        IUnknown_AddRef(iface);
        *obj = &This->IDirect3DVertexBuffer_iface;
        TRACE("  Creating IDirect3DVertexBuffer interface %p\n", *obj);
        return S_OK;
    }
    if ( IsEqualGUID( &IID_IDirect3DVertexBuffer7, riid ) )
    {
        IUnknown_AddRef(iface);
        *obj = iface;
        TRACE("  Creating IDirect3DVertexBuffer7 interface %p\n", *obj);
        return S_OK;
    }
    FIXME("(%p): interface for IID %s NOT found!\n", This, debugstr_guid(riid));
    return E_NOINTERFACE;
}

static HRESULT WINAPI IDirect3DVertexBufferImpl_1_QueryInterface(IDirect3DVertexBuffer *iface,
        REFIID riid, void **obj)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);

    TRACE("iface %p, riid %s, object %p.\n", iface, debugstr_guid(riid), obj);

    return IDirect3DVertexBuffer7_QueryInterface(&This->IDirect3DVertexBuffer7_iface, riid, obj);
}

/*****************************************************************************
 * IDirect3DVertexBuffer7::AddRef
 *
 * AddRef for Vertex Buffers
 *
 * Returns:
 *  The new refcount
 *
 *****************************************************************************/
static ULONG WINAPI IDirect3DVertexBufferImpl_AddRef(IDirect3DVertexBuffer7 *iface)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);
    ULONG ref = InterlockedIncrement(&This->ref);

    TRACE("%p increasing refcount to %u.\n", This, ref);

    return ref;
}

static ULONG WINAPI IDirect3DVertexBufferImpl_1_AddRef(IDirect3DVertexBuffer *iface)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);

    TRACE("iface %p.\n", iface);

    return IDirect3DVertexBuffer7_AddRef(&This->IDirect3DVertexBuffer7_iface);
}


/*****************************************************************************
 * IDirect3DVertexBuffer7::Release
 *
 * Release for Vertex Buffers
 *
 * Returns:
 *  The new refcount
 *
 *****************************************************************************/
static ULONG WINAPI IDirect3DVertexBufferImpl_Release(IDirect3DVertexBuffer7 *iface)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);
    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("%p decreasing refcount to %u.\n", This, ref);

    if (ref == 0)
    {
        struct wined3d_buffer *curVB = NULL;
        UINT offset, stride;

        /* D3D7 Vertex buffers don't stay bound in the device, they are passed
         * as a parameter to drawPrimitiveVB. DrawPrimitiveVB sets them as the
         * stream source in wined3d, and they should get unset there before
         * they are destroyed. */
        wined3d_mutex_lock();
        wined3d_device_get_stream_source(This->ddraw->wined3d_device,
                0, &curVB, &offset, &stride);
        if (curVB == This->wineD3DVertexBuffer)
            wined3d_device_set_stream_source(This->ddraw->wined3d_device, 0, NULL, 0, 0);
        if (curVB)
            wined3d_buffer_decref(curVB); /* For the GetStreamSource */

        wined3d_vertex_declaration_decref(This->wineD3DVertexDeclaration);
        wined3d_buffer_decref(This->wineD3DVertexBuffer);
        wined3d_mutex_unlock();

        HeapFree(GetProcessHeap(), 0, This);

        return 0;
    }
    return ref;
}

static ULONG WINAPI IDirect3DVertexBufferImpl_1_Release(IDirect3DVertexBuffer *iface)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);

    TRACE("iface %p.\n", iface);

    return IDirect3DVertexBuffer7_Release(&This->IDirect3DVertexBuffer7_iface);
}

/*****************************************************************************
 * IDirect3DVertexBuffer Methods
 *****************************************************************************/

/*****************************************************************************
 * IDirect3DVertexBuffer7::Lock
 *
 * Locks the vertex buffer and returns a pointer to the vertex data
 * Locking vertex buffers is similar to locking surfaces, because Windows
 * uses surfaces to store vertex data internally (According to the DX sdk)
 *
 * Params:
 *  Flags: Locking flags. Relevant here are DDLOCK_READONLY, DDLOCK_WRITEONLY,
 *         DDLOCK_DISCARDCONTENTS and DDLOCK_NOOVERWRITE.
 *  Data:  Returns a pointer to the vertex data
 *  Size:  Returns the size of the buffer if not NULL
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if Data is NULL
 *  D3DERR_VERTEXBUFFEROPTIMIZED if called on an optimized buffer(WineD3D)
 *
 *****************************************************************************/
static HRESULT WINAPI IDirect3DVertexBufferImpl_Lock(IDirect3DVertexBuffer7 *iface, DWORD Flags,
        void **Data, DWORD *Size)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);
    struct wined3d_resource_desc wined3d_desc;
    struct wined3d_resource *wined3d_resource;
    HRESULT hr;
    DWORD wined3d_flags = 0;

    TRACE("iface %p, flags %#x, data %p, data_size %p.\n", iface, Flags, Data, Size);

    /* Writeonly: Pointless. Event: Unsupported by native according to the sdk
     * nosyslock: Not applicable
     */
    if(!(Flags & DDLOCK_WAIT))          wined3d_flags |= WINED3DLOCK_DONOTWAIT;
    if(Flags & DDLOCK_READONLY)         wined3d_flags |= WINED3DLOCK_READONLY;
    if(Flags & DDLOCK_NOOVERWRITE)      wined3d_flags |= WINED3DLOCK_NOOVERWRITE;
    if(Flags & DDLOCK_DISCARDCONTENTS)  wined3d_flags |= WINED3DLOCK_DISCARD;

    wined3d_mutex_lock();
    if(Size)
    {
        /* Get the size, for returning it, and for locking */
        wined3d_resource = wined3d_buffer_get_resource(This->wineD3DVertexBuffer);
        wined3d_resource_get_desc(wined3d_resource, &wined3d_desc);
        *Size = wined3d_desc.size;
    }

    hr = wined3d_buffer_map(This->wineD3DVertexBuffer, 0, 0, (BYTE **)Data, wined3d_flags);
    wined3d_mutex_unlock();

    return hr;
}

static HRESULT WINAPI IDirect3DVertexBufferImpl_1_Lock(IDirect3DVertexBuffer *iface, DWORD Flags,
        void **Data, DWORD *Size)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);

    TRACE("iface %p, flags %#x, data %p, data_size %p.\n", iface, Flags, Data, Size);

    return IDirect3DVertexBuffer7_Lock(&This->IDirect3DVertexBuffer7_iface, Flags, Data, Size);
}

/*****************************************************************************
 * IDirect3DVertexBuffer7::Unlock
 *
 * Unlocks a vertex Buffer
 *
 * Returns:
 *  D3D_OK on success
 *
 *****************************************************************************/
static HRESULT WINAPI IDirect3DVertexBufferImpl_Unlock(IDirect3DVertexBuffer7 *iface)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);

    TRACE("iface %p.\n", iface);

    wined3d_mutex_lock();
    wined3d_buffer_unmap(This->wineD3DVertexBuffer);
    wined3d_mutex_unlock();

    return D3D_OK;
}

static HRESULT WINAPI IDirect3DVertexBufferImpl_1_Unlock(IDirect3DVertexBuffer *iface)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);

    TRACE("iface %p.\n", iface);

    return IDirect3DVertexBuffer7_Unlock(&This->IDirect3DVertexBuffer7_iface);
}


/*****************************************************************************
 * IDirect3DVertexBuffer7::ProcessVertices
 *
 * Processes untransformed Vertices into a transformed or optimized vertex
 * buffer. It can also perform other operations, such as lighting or clipping
 *
 * Params
 *  VertexOp: Operation(s) to perform: D3DVOP_CLIP, _EXTENTS, _LIGHT, _TRANSFORM
 *  DestIndex: Index in the destination buffer(This), where the vertices are
 *             placed
 *  Count: Number of Vertices in the Source buffer to process
 *  SrcBuffer: Source vertex buffer
 *  SrcIndex: Index of the first vertex in the src buffer to process
 *  D3DDevice: Device to use for transformation
 *  Flags: 0 for default, D3DPV_DONOTCOPYDATA to prevent copying
 *         unchaned vertices
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS If D3DVOP_TRANSFORM wasn't passed
 *
 *****************************************************************************/
static HRESULT WINAPI IDirect3DVertexBufferImpl_ProcessVertices(IDirect3DVertexBuffer7 *iface,
        DWORD VertexOp, DWORD DestIndex, DWORD Count, IDirect3DVertexBuffer7 *SrcBuffer,
        DWORD SrcIndex, IDirect3DDevice7 *device, DWORD Flags)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);
    IDirect3DVertexBufferImpl *Src = unsafe_impl_from_IDirect3DVertexBuffer7(SrcBuffer);
    IDirect3DDeviceImpl *device_impl = unsafe_impl_from_IDirect3DDevice7(device);
    BOOL oldClip, doClip;
    HRESULT hr;

    TRACE("iface %p, vertex_op %#x, dst_idx %u, count %u, src_buffer %p, src_idx %u, device %p, flags %#x.\n",
            iface, VertexOp, DestIndex, Count, SrcBuffer, SrcIndex, device, Flags);

    /* Vertex operations:
     * D3DVOP_CLIP: Clips vertices outside the viewing frustrum. Needs clipping information
     * in the vertex buffer (Buffer may not be created with D3DVBCAPS_DONOTCLIP)
     * D3DVOP_EXTENTS: Causes the screen extents to be updated when rendering the vertices
     * D3DVOP_LIGHT: Lights the vertices
     * D3DVOP_TRANSFORM: Transform the vertices. This flag is necessary
     *
     * WineD3D only transforms and clips the vertices by now, so EXTENTS and LIGHT
     * are not implemented. Clipping is disabled ATM, because of unsure conditions.
     */
    if( !(VertexOp & D3DVOP_TRANSFORM) ) return DDERR_INVALIDPARAMS;

    wined3d_mutex_lock();

    /* WineD3D doesn't know d3d7 vertex operation, it uses
     * render states instead. Set the render states according to
     * the vertex ops
     */
    doClip = VertexOp & D3DVOP_CLIP ? TRUE : FALSE;
    wined3d_device_get_render_state(device_impl->wined3d_device, WINED3D_RS_CLIPPING, (DWORD *)&oldClip);
    if (doClip != oldClip)
        wined3d_device_set_render_state(device_impl->wined3d_device, WINED3D_RS_CLIPPING, doClip);

    wined3d_device_set_stream_source(device_impl->wined3d_device,
            0, Src->wineD3DVertexBuffer, 0, get_flexible_vertex_size(Src->fvf));
    wined3d_device_set_vertex_declaration(device_impl->wined3d_device, Src->wineD3DVertexDeclaration);
    hr = wined3d_device_process_vertices(device_impl->wined3d_device, SrcIndex, DestIndex,
            Count, This->wineD3DVertexBuffer, NULL, Flags, This->fvf);

    /* Restore the states if needed */
    if (doClip != oldClip)
        wined3d_device_set_render_state(device_impl->wined3d_device, WINED3D_RS_CLIPPING, oldClip);

    wined3d_mutex_unlock();

    return hr;
}

static HRESULT WINAPI IDirect3DVertexBufferImpl_1_ProcessVertices(IDirect3DVertexBuffer *iface,
        DWORD VertexOp, DWORD DestIndex, DWORD Count, IDirect3DVertexBuffer *SrcBuffer,
        DWORD SrcIndex, IDirect3DDevice3 *device, DWORD Flags)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);
    IDirect3DVertexBufferImpl *Src = unsafe_impl_from_IDirect3DVertexBuffer(SrcBuffer);
    IDirect3DDeviceImpl *device_impl = unsafe_impl_from_IDirect3DDevice3(device);

    TRACE("iface %p, vertex_op %#x, dst_idx %u, count %u, src_buffer %p, src_idx %u, device %p, flags %#x.\n",
            iface, VertexOp, DestIndex, Count, SrcBuffer, SrcIndex, device, Flags);

    return IDirect3DVertexBuffer7_ProcessVertices(&This->IDirect3DVertexBuffer7_iface, VertexOp,
            DestIndex, Count, &Src->IDirect3DVertexBuffer7_iface, SrcIndex,
            device_impl ? &device_impl->IDirect3DDevice7_iface : NULL, Flags);
}

/*****************************************************************************
 * IDirect3DVertexBuffer7::GetVertexBufferDesc
 *
 * Returns the description of a vertex buffer
 *
 * Params:
 *  Desc: Address to write the description to
 *
 * Returns
 *  DDERR_INVALIDPARAMS if Desc is NULL
 *  D3D_OK on success
 *
 *****************************************************************************/
static HRESULT WINAPI IDirect3DVertexBufferImpl_GetVertexBufferDesc(IDirect3DVertexBuffer7 *iface,
        D3DVERTEXBUFFERDESC *Desc)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);
    struct wined3d_resource_desc wined3d_desc;
    struct wined3d_resource *wined3d_resource;

    TRACE("iface %p, desc %p.\n", iface, Desc);

    if(!Desc) return DDERR_INVALIDPARAMS;

    wined3d_mutex_lock();
    wined3d_resource = wined3d_buffer_get_resource(This->wineD3DVertexBuffer);
    wined3d_resource_get_desc(wined3d_resource, &wined3d_desc);
    wined3d_mutex_unlock();

    /* Now fill the Desc structure */
    Desc->dwCaps = This->Caps;
    Desc->dwFVF = This->fvf;
    Desc->dwNumVertices = wined3d_desc.size / get_flexible_vertex_size(This->fvf);

    return D3D_OK;
}

static HRESULT WINAPI IDirect3DVertexBufferImpl_1_GetVertexBufferDesc(IDirect3DVertexBuffer *iface,
        D3DVERTEXBUFFERDESC *Desc)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);

    TRACE("iface %p, desc %p.\n", iface, Desc);

    return IDirect3DVertexBuffer7_GetVertexBufferDesc(&This->IDirect3DVertexBuffer7_iface, Desc);
}


/*****************************************************************************
 * IDirect3DVertexBuffer7::Optimize
 *
 * Converts an unoptimized vertex buffer into an optimized buffer
 *
 * Params:
 *  D3DDevice: Device for which this buffer is optimized
 *  Flags: Not used, should be set to 0
 *
 * Returns
 *  D3D_OK, because it's a stub
 *
 *****************************************************************************/
static HRESULT WINAPI IDirect3DVertexBufferImpl_Optimize(IDirect3DVertexBuffer7 *iface,
        IDirect3DDevice7 *D3DDevice, DWORD Flags)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer7(iface);
    static BOOL hide = FALSE;

    TRACE("iface %p, device %p, flags %#x.\n", iface, D3DDevice, Flags);

    if (!hide)
    {
        FIXME("iface %p, device %p, flags %#x stub!\n", iface, D3DDevice, Flags);
        hide = TRUE;
    }

    /* We could forward this call to WineD3D and take advantage
     * of it once we use OpenGL vertex buffers
     */
    wined3d_mutex_lock();
    This->Caps |= D3DVBCAPS_OPTIMIZED;
    wined3d_mutex_unlock();

    return DD_OK;
}

static HRESULT WINAPI IDirect3DVertexBufferImpl_1_Optimize(IDirect3DVertexBuffer *iface,
        IDirect3DDevice3 *device, DWORD Flags)
{
    IDirect3DVertexBufferImpl *This = impl_from_IDirect3DVertexBuffer(iface);
    IDirect3DDeviceImpl *device_impl = unsafe_impl_from_IDirect3DDevice3(device);

    TRACE("iface %p, device %p, flags %#x.\n", iface, device, Flags);

    return IDirect3DVertexBuffer7_Optimize(&This->IDirect3DVertexBuffer7_iface,
            device_impl ? &device_impl->IDirect3DDevice7_iface : NULL, Flags);
}

/*****************************************************************************
 * IDirect3DVertexBuffer7::ProcessVerticesStrided
 *
 * This method processes untransformed strided vertices into a processed
 * or optimized vertex buffer.
 *
 * For more details on the parameters, see
 * IDirect3DVertexBuffer7::ProcessVertices
 *
 * Params:
 *  VertexOp: Operations to perform
 *  DestIndex: Destination index to write the vertices to
 *  Count: Number of input vertices
 *  StrideData: Array containing the input vertices
 *  VertexTypeDesc: Vertex Description or source index?????????
 *  D3DDevice: IDirect3DDevice7 to use for processing
 *  Flags: Can be D3DPV_DONOTCOPYDATA to avoid copying unmodified vertices
 *
 * Returns
 *  D3D_OK on success, or DDERR_*
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DVertexBufferImpl_ProcessVerticesStrided(IDirect3DVertexBuffer7 *iface,
                                                 DWORD VertexOp,
                                                 DWORD DestIndex,
                                                 DWORD Count,
                                                 D3DDRAWPRIMITIVESTRIDEDDATA *StrideData,
                                                 DWORD VertexTypeDesc,
                                                 IDirect3DDevice7 *D3DDevice,
                                                 DWORD Flags)
{
    FIXME("iface %p, vertex_op %#x, dst_idx %u, count %u, data %p, vertex_type %#x, device %p, flags %#x stub!\n",
            iface, VertexOp, DestIndex, Count, StrideData, VertexTypeDesc, D3DDevice, Flags);

    return DD_OK;
}

/*****************************************************************************
 * The VTables
 *****************************************************************************/

static const struct IDirect3DVertexBuffer7Vtbl d3d_vertex_buffer7_vtbl =
{
    /*** IUnknown Methods ***/
    IDirect3DVertexBufferImpl_QueryInterface,
    IDirect3DVertexBufferImpl_AddRef,
    IDirect3DVertexBufferImpl_Release,
    /*** IDirect3DVertexBuffer Methods ***/
    IDirect3DVertexBufferImpl_Lock,
    IDirect3DVertexBufferImpl_Unlock,
    IDirect3DVertexBufferImpl_ProcessVertices,
    IDirect3DVertexBufferImpl_GetVertexBufferDesc,
    IDirect3DVertexBufferImpl_Optimize,
    /*** IDirect3DVertexBuffer7 Methods ***/
    IDirect3DVertexBufferImpl_ProcessVerticesStrided
};

static const struct IDirect3DVertexBufferVtbl d3d_vertex_buffer1_vtbl =
{
    /*** IUnknown Methods ***/
    IDirect3DVertexBufferImpl_1_QueryInterface,
    IDirect3DVertexBufferImpl_1_AddRef,
    IDirect3DVertexBufferImpl_1_Release,
    /*** IDirect3DVertexBuffer Methods ***/
    IDirect3DVertexBufferImpl_1_Lock,
    IDirect3DVertexBufferImpl_1_Unlock,
    IDirect3DVertexBufferImpl_1_ProcessVertices,
    IDirect3DVertexBufferImpl_1_GetVertexBufferDesc,
    IDirect3DVertexBufferImpl_1_Optimize
};

HRESULT d3d_vertex_buffer_create(IDirect3DVertexBufferImpl **vertex_buf, struct ddraw *ddraw,
        D3DVERTEXBUFFERDESC *desc)
{
    IDirect3DVertexBufferImpl *buffer;
    DWORD usage;
    HRESULT hr = D3D_OK;

    TRACE("Vertex buffer description:\n");
    TRACE("    dwSize %u\n", desc->dwSize);
    TRACE("    dwCaps %#x\n", desc->dwCaps);
    TRACE("    FVF %#x\n", desc->dwFVF);
    TRACE("    dwNumVertices %u\n", desc->dwNumVertices);

    buffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*buffer));
    if (!buffer)
        return DDERR_OUTOFMEMORY;

    buffer->IDirect3DVertexBuffer7_iface.lpVtbl = &d3d_vertex_buffer7_vtbl;
    buffer->IDirect3DVertexBuffer_iface.lpVtbl = &d3d_vertex_buffer1_vtbl;
    buffer->ref = 1;

    buffer->ddraw = ddraw;
    buffer->Caps = desc->dwCaps;
    buffer->fvf = desc->dwFVF;

    usage = desc->dwCaps & D3DVBCAPS_WRITEONLY ? WINED3DUSAGE_WRITEONLY : 0;
    usage |= WINED3DUSAGE_STATICDECL;

    wined3d_mutex_lock();

    hr = wined3d_buffer_create_vb(ddraw->wined3d_device,
            get_flexible_vertex_size(desc->dwFVF) * desc->dwNumVertices,
            usage, desc->dwCaps & D3DVBCAPS_SYSTEMMEMORY ? WINED3D_POOL_SYSTEM_MEM : WINED3D_POOL_DEFAULT,
            buffer, &ddraw_null_wined3d_parent_ops, &buffer->wineD3DVertexBuffer);
    if (FAILED(hr))
    {
        WARN("Failed to create wined3d vertex buffer, hr %#x.\n", hr);
        if (hr == WINED3DERR_INVALIDCALL)
            hr = DDERR_INVALIDPARAMS;
        goto end;
    }

    buffer->wineD3DVertexDeclaration = ddraw_find_decl(ddraw, desc->dwFVF);
    if (!buffer->wineD3DVertexDeclaration)
    {
        ERR("Failed to find vertex declaration for fvf %#x.\n", desc->dwFVF);
        wined3d_buffer_decref(buffer->wineD3DVertexBuffer);
        hr = DDERR_INVALIDPARAMS;
        goto end;
    }
    wined3d_vertex_declaration_incref(buffer->wineD3DVertexDeclaration);

end:
    wined3d_mutex_unlock();
    if (hr == D3D_OK)
        *vertex_buf = buffer;
    else
        HeapFree(GetProcessHeap(), 0, buffer);

    return hr;
}

IDirect3DVertexBufferImpl *unsafe_impl_from_IDirect3DVertexBuffer(IDirect3DVertexBuffer *iface)
{
    if (!iface)
        return NULL;
    assert(iface->lpVtbl == &d3d_vertex_buffer1_vtbl);

    return impl_from_IDirect3DVertexBuffer(iface);
}

IDirect3DVertexBufferImpl *unsafe_impl_from_IDirect3DVertexBuffer7(IDirect3DVertexBuffer7 *iface)
{
    if (!iface)
        return NULL;
    assert(iface->lpVtbl == &d3d_vertex_buffer7_vtbl);

    return impl_from_IDirect3DVertexBuffer7(iface);
}
