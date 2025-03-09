#pragma once

#include <utils/win.hpp>

#include <d3d8.h>
#include <atlbase.h>

#pragma comment(lib, "d3d8.lib")

struct __declspec(uuid("1dd9e8da-1c77-4d40-b0cf-98fefdff9512")) IDirect3D8;
struct __declspec(uuid("7385e5df-8fe8-41d5-86b6-d7b48547b6cf")) IDirect3DDevice8;
struct __declspec(uuid("1b36bb7b-09b7-410a-b445-7d1430d7b33f")) IDirect3DResource8;
struct __declspec(uuid("b4211cfa-51b9-4a9f-ab78-db99b2bb678e")) IDirect3DBaseTexture8;
struct __declspec(uuid("e4cdd575-2866-4f01-b12e-7eece1ec9358")) IDirect3DTexture8;
struct __declspec(uuid("3ee5b968-2aca-4c34-8bb5-7e0c3d19b750")) IDirect3DCubeTexture8;
struct __declspec(uuid("4b8aaafa-140f-42ba-9131-597eafaa2ead")) IDirect3DVolumeTexture8;
struct __declspec(uuid("8aeeeac7-05f9-44d4-b591-000b0df1cb95")) IDirect3DVertexBuffer8;
struct __declspec(uuid("0e689c9a-053d-44a0-9d92-db0e3d750f86")) IDirect3DIndexBuffer8;
struct __declspec(uuid("b96eebca-b326-4ea5-882f-2ff5bae021dd")) IDirect3DSurface8;
struct __declspec(uuid("bd7349f5-14f1-42e4-9c79-972380db40c0")) IDirect3DVolume8;
struct __declspec(uuid("928c088b-76b9-4c6b-a536-a590853876cd")) IDirect3DSwapChain8;
