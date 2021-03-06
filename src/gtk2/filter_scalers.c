//
// filter_scalers.c
//
//
// ========================
//
// Initializes scalers, applies them,
// some utility functions, resource cleanup
//
// ========================

#include "filter_scalers.h"

// Filter includes
#include "../libris.h"

static scaler_info scalers[SCALER_ENUM_LAST];

static scaled_output_info scaled_output;
static gint scaler_mode;
static gint scaler_factor;
static gint scaler_factor_index;

gint scaler_size(gint scale_factor_new, gint scale_direction_new, gint size_current)
{
    gint size_new;
    if (scale_direction_new < 0)
    {
        size_new  = (size_current + scale_factor_new - 1) / scale_factor_new;
    } else {
        size_new  = size_current  * scale_factor_new;
    }
    return size_new;
}

// scaler_name_get
//
// Returns string name of a scaler
//
// scaler_index: desired scaler (from the enum scaler_list)
//
const char * scaler_name_get(gint scaler_index)
{
    return (scalers[scaler_index].scaler_name);
}

const char * scaler_namevalue_get(gint index)
{
    return (scalers[SCALER_ENUM_FIRST].scale[index].name);
}

// scaler_mode_set
//
// Sets the current scaler mode
// Preserves current mode across different callers
// (filter_dialog.c,  filter_pixel_art_scalers.c)
//
void scaler_mode_set(gint scaler_mode_new)
{
    // Don't update the mode if it's outside the allowed range
    if ((scaler_mode_new >= SCALER_ENUM_FIRST) && (scaler_mode_new < SCALER_ENUM_LAST))
        scaler_mode = scaler_mode_new;
}

// scaler_mode_get
//
// Returns the current scaler mode
//
gint scaler_mode_get(void)
{
    return (scaler_mode);
}

// scaler_factor_set
//
// Sets the current scaler mode
// Preserves current mode across different callers
// (filter_dialog.c,  filter_pixel_art_scalers.c)
//
void scaler_factor_set(gint scaler_factor_new)
{
    // Don't update the mode if it's outside the allowed range
    if ((scaler_factor_new >= SCALEV_ENUM_FIRST) && (scaler_factor_new < SCALEV_ENUM_LAST))
        scaler_factor_index = scaler_factor_new;
}

// scaler_scale_factor_get
//
// Returns scale factor (x 2, x 3, etc) of a scaler
//
// scaler_index: desired scaler (from the enum scaler_list)
//
gint scaler_factor_index_get(void)
{
    return (scaler_factor_index);
}

gint scaler_factor_get(void)
{
    return (scalers[scaler_mode].scale[scaler_factor_index].factor);
}

gint scaler_direction_get(void)
{
    return (scalers[scaler_mode].direction);
}

// scaled_info_get
//
// Returns structure (type scaled_output_info)
// with details about the current rendered
// output image (scale mode, width, height, factor, etc)
//
// Used to assist with output caching
//
scaled_output_info * scaled_info_get(void)
{
    return &scaled_output;
}

// scaled_output_check_reapply_scalers
//
// Checks whether the scaler needs to be re-applied
// depending on whether the scaler mode or
// x/y source image offset location have changed
//
// Used to assist with output caching
//
gint scaled_output_check_reapply_scalers(gint scaler_mode_new, gint scale_factor_new, gint scale_direction_new, gint x_new, gint y_new)
{
    gint result;

    result = ((scaled_output.scaler_mode != scaler_mode_new) || (scaled_output.scale_factor != scale_factor_new) || (scaled_output.direction != scale_direction_new) || (scaled_output.x != x_new) || (scaled_output.y != y_new) || (scaled_output.valid_image == FALSE));

    scaled_output.x = x_new;
    scaled_output.y = y_new;

    return (result);
}

// scaled_output_check_reallocate
//
// Update output buffer size and re-allocate if needed
//
void scaled_output_check_reallocate(gint scale_factor_new, gint scale_direction_new, gint width_new, gint height_new)
{

    if ((scale_factor_new != scaled_output.scale_factor) || (scale_direction_new != scaled_output.direction) || ((width_new  * scale_factor_new) != scaled_output.width) || ((height_new * scale_factor_new) != scaled_output.height) || (scaled_output.p_scaledbuf == NULL))
    {

        // Update the buffer size and re-allocate. The x uint32_t is for RGBA buffer size
        scaled_output.direction    = scale_direction_new;
        scaled_output.scale_factor = scale_factor_new;
        scaled_output.width  = scaler_size(scale_factor_new, scale_direction_new, width_new);
        scaled_output.height = scaler_size(scale_factor_new, scale_direction_new, height_new);
        scaled_output.size_bytes   = scaled_output.width * scaled_output.height * BYTE_SIZE_RGBA_4BPP;

        if (scaled_output.p_scaledbuf)
            g_free(scaled_output.p_scaledbuf);

        // 32 bit to ensure alignment, divide size since it's in BYTES
        scaled_output.p_scaledbuf = (uint32_t *) g_new (guint32, scaled_output.size_bytes / BYTE_SIZE_RGBA_4BPP);

        // Invalidate the image
        scaled_output.valid_image = FALSE;
    }
}

// scaled_output_init
//
// Initialize rendered output shared structure
//
void scaled_output_init(void)
{
    scaled_output.p_scaledbuf  = NULL;
    scaled_output.width        = 0;
    scaled_output.height       = 0;
    scaled_output.x            = 0;
    scaled_output.y            = 0;
    scaled_output.scale_factor = 0;
    scaled_output.scaler_mode  = 0;
    scaled_output.direction    = 0;
    scaled_output.size_bytes   = 0;
    scaled_output.bpp          = 0;
    scaled_output.valid_image  = FALSE;
}

// scaler_apply
//
// Calls selected scaler function
// Updates valid_image to assist with caching
//
void scaler_apply(int scaler_mode, int scaler_factor_index, uint32_t * p_srcbuf, uint32_t * p_destbuf, int width, int height)
{
    if ((p_srcbuf == NULL) || (p_destbuf == NULL))
        return;

    if ((scaler_mode < SCALER_ENUM_LAST) && (scaler_factor_index < SCALEV_ENUM_LAST))
    {
        // Call the requested scaler function
        scalers[scaler_mode].scale[scaler_factor_index].function((uint32_t*) p_srcbuf, (uint32_t*) p_destbuf, (int) width, (int) height);
        scaled_output.bpp = BYTE_SIZE_RGBA_4BPP;
        scaled_output.scaler_mode = scaler_mode;
        scaled_output.scale_factor = scalers[scaler_mode].scale[scaler_factor_index].factor;
        scaled_output.valid_image = TRUE;
    }
}

// buffer_add_alpha_byte
//
// Utility function to convert 3BPP RGB to 4BPP RGBA
// by inserting a fourth (alpha channel) byte after every 3 bytes
//
void buffer_add_alpha_byte(guchar * p_srcbuf, glong srcbuf_size)
{

    // Iterates through the buffer backward, from END to START
    // and remaps from RGB to RGBA (adds alpha byte)
    // --> Copies the buffer on top of itself, so ORDER IS IMPORTANT!

    guchar * p_3bpp = p_srcbuf + ((srcbuf_size / 4) *3) - 3; // Last pixel of 3BPP buffer
    guchar * p_4bpp = p_srcbuf + srcbuf_size -  4;          // Last pixel of 4BPP Buffer
    glong idx = 0;

    while(idx < srcbuf_size)
    {
        p_4bpp[3] = ALPHA_MASK_OPAQUE;  // Set alpha mask byte to 100% opaque / visible
        p_4bpp[2] = p_3bpp[2]; // copy B
        p_4bpp[1] = p_3bpp[1]; // copy G
        p_4bpp[0] = p_3bpp[0]; // copy R

        p_3bpp -= 3;  // Advance 3BPP image pointer to previous pixel
        p_4bpp -= 4;  // Advance 4BPP image pointer to previous pixel
        idx += 4;
    }
}

// buffer_remove_alpha_byte
//
// Utility function to convert 4BPP RGBA to 3BPP RGB
// by removing the fourth (alpha channel) byte after every 3 bytes
//
void buffer_remove_alpha_byte(guchar * p_srcbuf, glong srcbuf_size)
{

    // Iterates through the buffer forward, from START to END
    // and remaps from RGBA to RGB (removes alpha byte)
    // --> Copies the buffer on top of itself, so ORDER IS IMPORTANT!

    guchar * p_3bpp = p_srcbuf; // Last First of 3BPP buffer
    guchar * p_4bpp = p_srcbuf; // Last First of 4BPP Buffer
    glong idx = 0;

    while(idx < srcbuf_size)
    {
        p_3bpp[0] = p_4bpp[0]; // copy R
        p_3bpp[1] = p_4bpp[1]; // copy G
        p_3bpp[2] = p_4bpp[2]; // copy B

        p_3bpp += 3;  // Advance 3BPP image pointer to next pixel
        p_4bpp += 4;  // Advance 4BPP image pointer to next pixel
        idx += 4;
    }
}

// pixel_art_scalers_release_resources
//
// Release the scaled output buffer.
// Should be called only at the very
// end of the plugin shutdown (not on dialog close)
//
void pixel_art_scalers_release_resources(void)
{

    if (scaled_output.p_scaledbuf)
        g_free(scaled_output.p_scaledbuf);
}

void scaler_image_copy(uint32_t *sp, uint32_t *dp, int Xres, int Yres, int scale_factor)
{
    int       x, y, sx;

    for (y=0; y < Yres; y++)
    {
        // Copy each line from the source image
        for (x=0; x < Xres; x++)
        {

            // Copy new pixels from left source pixel
            for (sx = 0; sx < scale_factor; sx++)
            {
                // Copy each uint32 (RGBA 4BPP) pixel to the dest buffer
                *dp++ = *sp;
            }
            // Move to next source pixel
            sp++;
        }
    }
}

// scalers_init
//
// Populate the shared list of available scalers with their names
// calling functions and scale factors.
//
void scalers_init(void)
{
    int index;
    scaled_output_init();

    // HRIS
    index = SCALER_HRIS;
    scalers[index].scale[SCALEV_2X].function = &scaler_hris_2x;
    scalers[index].scale[SCALEV_2X].factor = 2;
    snprintf(scalers[index].scale[SCALEV_2X].name, SCALER_STR_MAX, "2X");
    scalers[index].scale[SCALEV_3X].function = &scaler_hris_3x;
    scalers[index].scale[SCALEV_3X].factor = 3;
    snprintf(scalers[index].scale[SCALEV_3X].name, SCALER_STR_MAX, "3X");
    snprintf(scalers[index].scaler_name, SCALER_STR_MAX, "HRIS");
    scalers[index].direction = 1;

    // GSAMPLE
    index = SCALER_GSAMPLE;
    scalers[index].scale[SCALEV_2X].function = &gsample_2x;
    scalers[index].scale[SCALEV_2X].factor = 2;
    snprintf(scalers[index].scale[SCALEV_2X].name, SCALER_STR_MAX, "2X");
    scalers[index].scale[SCALEV_3X].function = &gsample_3x;
    scalers[index].scale[SCALEV_3X].factor = 3;
    snprintf(scalers[index].scale[SCALEV_3X].name, SCALER_STR_MAX, "3X");
    snprintf(scalers[index].scaler_name, SCALER_STR_MAX, "Gsample");
    scalers[index].direction = 1;

    // MEAN
    index = SCALER_MEAN;
    scalers[index].scale[SCALEV_2X].function = &scaler_mean_2x;
    scalers[index].scale[SCALEV_2X].factor = 2;
    snprintf(scalers[index].scale[SCALEV_2X].name, SCALER_STR_MAX, "2X");
    scalers[index].scale[SCALEV_3X].function = &scaler_mean_3x;
    scalers[index].scale[SCALEV_3X].factor = 3;
    snprintf(scalers[index].scale[SCALEV_3X].name, SCALER_STR_MAX, "3X");
    snprintf(scalers[index].scaler_name, SCALER_STR_MAX, "Mean");
    scalers[index].direction = -1;

    // Now set the default scaler
    scaler_mode = SCALER_HRIS;
    scaler_factor_index = SCALEV_2X;
}
