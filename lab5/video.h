#pragma pack(1)

typedef struct {
  uint8_t vbe_signature[4];
  uint8_t vbe_version[2];
  phys_bytes oem_string_ptr;
  uint8_t capabilities[4];
  phys_bytes mode_list_ptr;
  uint16_t total_memory;
  uint16_t oem_software_rev;
  phys_bytes oem_vendor_name_ptr;
  phys_bytes oem_product_name_ptr;
  phys_bytes oem_product_rev_ptr;
  uint8_t reserved[222];
  uint8_t oem_data[256];
} VbeControllerInfo;

unsigned int get_h_res();

unsigned int get_v_res();

unsigned int get_bits_per_pixel();

bool isValidMode(vg_vbe_contr_info_t * cont_info, uint16_t mode);

uint32_t convert_far_pointer(phys_bytes pointer, void * base);

int parse_vbe_controller(vg_vbe_contr_info_t *controller_info, void * base_pointer);

int parse_vbe_mode(uint16_t mode, vbe_mode_info_t *mode_info);

void *(vg_init)(uint16_t mode);

int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       uint32_t color);

int delete_pixel(uint16_t x, uint16_t y);
