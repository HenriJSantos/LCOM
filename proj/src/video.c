#include <machine/int86.h>
#include <lcom/lcf.h>
#include <math.h>

#include "video.h"
#include "UTIL.h"
#include "VIDEO_MACRO.h"

static void *video_mem;         /* Process (virtual) address to which VRAM is mapped */
static void *buffer;

static unsigned h_res;          /* Horizontal resolution in pixels */
static unsigned v_res;          /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */
static unsigned vram_size;

void * get_video_mem()
{
    return video_mem;
}

void free_buffer()
{
    free(buffer);
}

void * get_buffer()
{
    return buffer;
}

unsigned int get_h_res()
{
    return h_res;
}

unsigned int get_v_res()
{
    return v_res;
}

unsigned int get_bits_per_pixel()
{
    return bits_per_pixel;
}

uint32_t convert_far_virtual(phys_bytes pointer, void * base)
{
    return (((pointer >> 16)<< 4) + LSB2(pointer) + (uint32_t) base);
}

void updateFrame() {
	memcpy(video_mem, buffer, vram_size);
}

bool isValidMode(vg_vbe_contr_info_t * cont_info, uint16_t mode)
{
  bool found = false;
  for (unsigned int i = 0; ((uint16_t *)cont_info->VideoModeList)[i] != 0xFFFF; i++)
  {
    if (((uint16_t *)cont_info->VideoModeList)[i] == mode)
    {
      found = true;
      break;
    }
  }
  return found;
}

int parse_vbe_controller(vg_vbe_contr_info_t * controller_info, void * base_pointer)
{
    struct reg86u rg;
    memset(&rg, 0, sizeof(rg));

    mmap_t map;
    if (lm_alloc(sizeof(VbeControllerInfo), &map) == NULL)
    {
        printf("Couldn't allocate necessary memory.\n");
        return 1;
    }

    VbeControllerInfo vbe_info;
    memcpy((VbeControllerInfo *)map.virt, "VBE2", 4);

    rg.u.w.ax = GET_CONTROLLER_INFO; // VBE call, function 00 -- read Controller info
    rg.u.w.es = PB2BASE(map.phys);
    rg.u.w.di = PB2OFF(map.phys);
    rg.u.b.intno = VC_INT_VECTOR;

    if (sys_int86(&rg) != OK)
    {
        switch (rg.u.b.ah)
        {
        case 1:
            printf("Function call failed.\n");
        case 2:
            printf("Function is not supported in the current hardware configuration\n");
        case 3:
            printf("Function call invalid in current video mode\n");
        }
        lm_free(&map);
        return 2;
    }

    vbe_info = *(VbeControllerInfo *)map.virt;
    memcpy(controller_info->VBESignature, vbe_info.vbe_signature, 4);
    memcpy(controller_info->VBEVersion, vbe_info.vbe_version, 2);
    controller_info->OEMString = (char *)convert_far_virtual(vbe_info.oem_string_ptr, base_pointer);
    controller_info->VideoModeList = (uint16_t *)convert_far_virtual(vbe_info.mode_list_ptr, base_pointer);
    controller_info->TotalMemory = vbe_info.total_memory * 64;
    controller_info->OEMVendorNamePtr = (char *)convert_far_virtual(vbe_info.oem_vendor_name_ptr, base_pointer);
    controller_info->OEMProductNamePtr = (char *)convert_far_virtual(vbe_info.oem_product_name_ptr, base_pointer);
    controller_info->OEMProductRevPtr = (char *)convert_far_virtual(vbe_info.oem_product_rev_ptr, base_pointer);
    lm_free(&map);
    return 0;
}

int parse_vbe_mode(uint16_t mode, vbe_mode_info_t *mode_info)
{
    struct reg86u rg;
    memset(&rg, 0, sizeof(rg));

    mmap_t map;
    if (lm_alloc(sizeof(vbe_mode_info_t), &map) == NULL)
    {
        printf("Couldn't allocate necessary memory.\n");
        return 1;
    }

    rg.u.w.ax = GET_VIDEO_MODE; // VBE call, function 02 -- set VBE mode
    rg.u.w.cx = mode;           // set bit 14: linear framebuffer
    rg.u.w.es = PB2BASE(map.phys);
    rg.u.w.di = PB2OFF(map.phys);
    rg.u.b.intno = VC_INT_VECTOR;

    if (sys_int86(&rg) != OK)
    {
        switch (rg.u.b.ah)
        {
        case 1:
            printf("Function call failed.\n");
        case 2:
            printf("Function is not supported in the current hardware configuration\n");
        case 3:
            printf("Function call invalid in current video mode\n");
        }
        lm_free(&map);
        return 2;
    }

    *mode_info = *(vbe_mode_info_t *)map.virt;
    lm_free(&map);
    return 0;
}

void *(vg_init)(uint16_t mode)
{
    vbe_mode_info_t info;
    if(parse_vbe_mode(mode, &info))
        return NULL;

    h_res = info.XResolution;
    v_res = info.YResolution;
    bits_per_pixel = info.BitsPerPixel;

    int r;
    struct minix_mem_range mr;
    unsigned int vram_base = info.PhysBasePtr;                                 /* VRAM's physical addresss */
    vram_size = h_res * v_res * ceil(bits_per_pixel / (double)8); /* VRAM's size, but you can use the frame-buffer size, instead */

    /* Allow memory mapping */
    mr.mr_base = (phys_bytes) vram_base;
    mr.mr_limit = mr.mr_base + vram_size;

    if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);

    /* Map memory */
    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

    if(video_mem == MAP_FAILED)
        panic("couldn’t map video memory");

    struct reg86u rg;
    memset(&rg, 0, sizeof(rg));

    rg.u.w.ax = SET_VIDEO_MODE; // VBE call, function 02 -- set VBE mode
    rg.u.w.bx = BIT(14) | mode; // set bit 14: linear framebuffer
    rg.u.b.intno = VC_INT_VECTOR;

    if (sys_int86(&rg) != OK)
    {
        printf("Couldn't initalize specified mode.");
        return NULL;
    }

    buffer = (char *) malloc(vram_size);

    return video_mem;
}

int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (x >= h_res || y >= v_res)
        return 1;
    unsigned int bytesUsed = (int)ceil(bits_per_pixel / (double)8);
    char *point = (char *)buffer + (y * h_res + x) * bytesUsed;
    for (uint8_t offset = 0; offset < bytesUsed; offset++, point++, color = color>> 8)
    {
        *point = color;
    }
    return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color)
{
    for (uint16_t currX = x; currX < x + len; currX++)
    {
        vg_draw_pixel(currX, y, color);
    }
    return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    for (uint16_t currY = y; currY < y + height; currY++)
    {
        vg_draw_hline(x, currY, width, color);
    }
    return 0;
}

int delete_pixel(uint16_t x, uint16_t y)
{
    uint32_t color=0;
    if (x >= h_res || y >= v_res)
        return 1;
    unsigned int bytesUsed = (int)ceil(bits_per_pixel / (double)8);
    char *point;
    for (uint8_t offset = 0; offset<bytesUsed; offset++, point++, color = color>> 8)
    {
        point = (char *)video_mem + (y * h_res + x) * bytesUsed + offset;
        *point = color;
    }
    return 0;
}
