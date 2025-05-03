<h1 align="center">
	Game Overlay
	<br>
	<a href="https://github.com/momo5502/gameoverlay?tab=GPL-2.0-1-ov-file"><img src="https://img.shields.io/github/license/momo5502/gameoverlay?color=00B0F8"/></a>
	<a href="https://github.com/momo5502/gameoverlay/actions"><img src="https://img.shields.io/github/actions/workflow/status/momo5502/gameoverlay/build.yml?branch=main&label=build"/></a>
	<a href="https://github.com/momo5502/gameoverlay/issues"><img src="https://img.shields.io/github/issues/momo5502/gameoverlay?color=F8B000"/></a>
	<img src="https://img.shields.io/github/commit-activity/m/momo5502/gameoverlay?color=FF3131"/>
</h1>

This project aims to provide a customizable gameoverlay using <a href="https://bitbucket.org/chromiumembedded/cef">CEF</a>.  
It is still unfinished and in a very early state.  
Hooking and drawing using common backends is already working,  
just like rendering offscreen browsers with CEF.

## Supported Backends

| Engine     | x86 | x64 |
|:---------- |:---:|:---:|
| DirectX 8  | ✅ |   |
| DirectX 9  | ✅ | ✅ |
| DirectX 10 | ✅ | ✅ |
| DirectX 11 | ✅ | ✅ |
| DirectX 12 | ✅ | ✅ |
| OpenGL     | ✅ | ✅ |
| Vulkan     | ⛔ | ⛔ |

## Preview

<img src="./docs/preview.jpg" />