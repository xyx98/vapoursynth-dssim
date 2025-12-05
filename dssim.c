#include <VapourSynth4.h>
#include <VSHelper4.h>

#include <dssim.h>

typedef struct {
    VSNode *reference;
    VSNode *distorted;
} dssimData;


static const VSFrame *VS_CC dssimGetFrame(int n, int activationReason, void *instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    dssimData *d = (dssimData *)instanceData;

    if (activationReason == arInitial) {
        // Request the source frame
        vsapi->requestFrameFilter(n, d->reference, frameCtx);
        vsapi->requestFrameFilter(n, d->distorted, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrame *reference = vsapi->getFrameFilter(n, d->reference, frameCtx);
        const VSFrame *distored = vsapi->getFrameFilter(n, d->distorted, frameCtx);
        const VSVideoFormat *fi = vsapi->getVideoFrameFormat(reference);
        int height = vsapi->getFrameHeight(reference, 0);
        int width = vsapi->getFrameWidth(reference, 0);

        VSMap *props = vsapi -> getFramePropertiesRW(distored);
        uint8_t *ref= (uint8_t *)malloc(sizeof(uint8_t)*width*height*3);
        uint8_t *dst= (uint8_t *)malloc(sizeof(uint8_t)*width*height*3);

        // prepare data for dssim
        for (int plane = 0; plane < fi->numPlanes; plane++) {
            const uint8_t * VS_RESTRICT refp = vsapi->getReadPtr(reference, plane);
            ptrdiff_t ref_stride = vsapi->getStride(reference, plane);
            uint8_t * VS_RESTRICT dstp = vsapi->getReadPtr(distored, plane);
            ptrdiff_t dst_stride = vsapi->getStride(distored, plane);

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    ref[3*(y*width+x)+plane]=refp[x];
                    dst[3*(y*width+x)+plane]=dstp[x];
                }
                dstp += dst_stride;
                refp += ref_stride;
            }
        }
        //apply dssim
        Dssim *dssim = dssim_new();
        DssimImage *dref=dssim_create_image_rgb(dssim,ref,width,height);
        DssimImage *ddst=dssim_create_image_rgb(dssim,dst,width,height);

        double dssim_score= dssim_compare(dssim,dref,ddst);
        double dssim_ssim=  1.0 / (1.0 + dssim_score);

        dssim_free_image(dref);
        dssim_free_image(ddst);
        dssim_free(dssim);

        vsapi->mapSetFloat(props,"dssim_score",dssim_score, maReplace);
        vsapi->mapSetFloat(props,"dssim_ssim",dssim_ssim, maReplace);
        // Release the source frame
        vsapi->freeFrame(reference);
        // Return the processed frame
        return distored;
    }

    return NULL;
}

// Free allocated data when the filter is destroyed
static void VS_CC dssimFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    dssimData *d = (dssimData *)instanceData;
    vsapi->freeNode(d->reference);
    vsapi->freeNode(d->distorted);
    free(d);
}

// Filter creation function: validates arguments and creates the filter
static void VS_CC dssimCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    dssimData d;
    dssimData *data;

    // Get the input clip
    d.reference = vsapi->mapGetNode(in, "reference", 0, nullptr);
    d.distorted = vsapi->mapGetNode(in, "distorted", 0, nullptr);

    const VSVideoInfo *vir = vsapi->getVideoInfo(d.reference);
    const VSVideoInfo *vid = vsapi->getVideoInfo(d.distorted);
    
    // Ensure the input format is RGB24
    if (!vsh_isConstantVideoFormat(vid) || !vsh_isSameVideoFormat(vir,vid) || vid->format.colorFamily != cfRGB || vid->format.sampleType != stInteger || vid->format.bitsPerSample != 8) {
        vsapi->mapSetError(out, "dssim: only RGB24 input supported");
        vsapi->freeNode(d.distorted);
        vsapi->freeNode(d.reference);
        return;
    }

    
    // Allocate memory for the filter data
    data = (dssimData *)malloc(sizeof(d));
    *data = d;

    // Register the filter
    VSFilterDependency deps[] = {{d.reference, rpStrictSpatial},{d.distorted, rpStrictSpatial}};
    vsapi->createVideoFilter(out, "dssim", vid, dssimGetFrame, dssimFree, fmParallel, deps, 1, data, core);
}

//////////////////////////////////////////
// Plugin Init

// This function is called when the plugin is loaded
VS_EXTERNAL_API(void) VapourSynthPluginInit2(VSPlugin *plugin, const VSPLUGINAPI *vspapi) {
    vspapi->configPlugin("com.xyx98.dssim", "dssim", "vapoursynth-dssim", VS_MAKE_VERSION(1, 0), VAPOURSYNTH_API_VERSION, 0, plugin);
    vspapi->registerFunction("dssim", "reference:vnode;distorted:vnode;", "clip:vnode;", dssimCreate, NULL, plugin);
}