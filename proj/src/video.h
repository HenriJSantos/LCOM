#pragma pack(1)

/** @defgroup Video Video
 * @{
 *
 * @brief Module for interaction with the video card
 */

/**
 * @brief Vbe Controller info header
 */
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

/**
 * @brief Get function for video memory
 * 
 * @return Returns video memory pointer
 */
void * get_video_mem();

/**
 * @brief Get function for buffer
 * 
 * @return Returns buffer pointer
 */
void * get_buffer();

/**
 * @brief Frees buffer memory
 */
void free_buffer();

/**
 * @brief Gets horizontal resolution
 * 
 * @return Screen's horizontal resolution
 */
unsigned int get_h_res();

/**
 * @brief Gets vertical resolution
 * 
 * @return Screen's vertical resolution
 */
unsigned int get_v_res();

/**
 * @brief Gets bits per pixel used
 * 
 * @return Bits per pixel used
 */
unsigned int get_bits_per_pixel();

/**
 * @brief Copies contents from buffer to video memory
 */
void updateFrame();

/**
 * @brief Checks if mode is valid
 * 
 * @param cont_info Struct that contains video controller's info
 * @param mode Mode to check if valid
 * 
 * @return true if mode is valid, false otherwise
 */
bool isValidMode(vg_vbe_contr_info_t * cont_info, uint16_t mode);

/**
 * @brief Converts physical pointer to far pointer
 * 
 * @param pointer Physical pointer to convert
 * @param base Base of virtual addressing memory
 * 
 * @return Virtual pointer equivalent of physical pointer
 */
uint32_t convert_far_pointer(phys_bytes pointer, void * base);

/**
 * @brief Parses VBE controller to a struct
 * 
 * @param controller_info Struct that contains video controller's info
 * @param base_pointer Base of virtual addressing memory
 * 
 * @return 0 if no errors occur, 1 otherwise
 */
int parse_vbe_controller(vg_vbe_contr_info_t *controller_info, void * base_pointer);

/**
 * @brief Parses VBE mode to a struct
 * 
 * @param mode Mode to parse
 * @param mode_info struct to store mode info
 * 
 * @return 0 if no errors occur, 1 otherwise
 */
int parse_vbe_mode(uint16_t mode, vbe_mode_info_t *mode_info);

/**
 * @brief Initializes minix video mode in pretended mode
 * 
 * @param mode Mode to change to
 */
void *(vg_init)(uint16_t mode);

/**
 * @brief Draws a pixel a certain color
 * 
 * @param x Horizontal position
 * @param y Vertical position
 * @param color Color to paint
 * 
 * @return 0 if no errors occur, 1 otherwise
 */
int vg_draw_pixel(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Draws a horizontal line a certain color
 * 
 * @param x Horizontal position to start
 * @param y Vertical position to start
 * @param len Length of line
 * @param color Color to paint
 * 
 * @return 0 if no errors occur, 1 otherwise
 */
int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

/**
 * @brief Draws a rectangle a certain color
 * 
 * @param x Horizontal position to start
 * @param y Vertical position to start
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param color Color to paint
 * 
 * @return 0 if no errors occur, 1 otherwise
 */
int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                       uint32_t color);

/**
 * @brief Paints a pixel black
 * 
 * @param x Horizontal position
 * @param y Vertical position
 * 
 * @return 0 if no errors occur, 1 otherwise
 */
int delete_pixel(uint16_t x, uint16_t y);
