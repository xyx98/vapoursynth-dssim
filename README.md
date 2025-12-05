# vapoursynth-dssim
Dssim for VapourSynth,based on [https://github.com/kornelski/dssim](https://github.com/kornelski/dssim).

## Usage
```python
dssim.dssim(vnode reference, vnode distorted)
# score stored as frame properties.
#'dssim_score' for original dssim score, means 1/SSIM-1.
#'dssim_ssim' for easy to use as normarl ssim score.
```
> [!CAUTION] 
> Just for learning write vapoursynth plugin.
> 
> Only RGB24 supported,it's dssim's c api limited (maybe).
> 
> It's slow.

## Build
> [!CAUTION] 
> Dssim c library should be installed before.
> 
> Read [dssim repository](https://github.com/kornelski/dssim?tab=readme-ov-file#usage-from-c) for help.
```shell
meson setup build
ninja -C build
```
