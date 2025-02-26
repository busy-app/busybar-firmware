#include "lv_fs.h"

#include <furi/furi.h>
#include <lvgl.h>
#include <storage/storage.h>

static lv_fs_res_t gui_lvgl_fs_convert_res(bool storage_res) {
    return storage_res ? LV_FS_RES_OK : LV_FS_RES_HW_ERR;
}

static void* gui_lvgl_fs_open(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
    Storage* storage = drv->user_data;

    File* file = storage_file_alloc(storage);
    FS_AccessMode access_mode = (mode == LV_FS_MODE_RD) ? FSAM_READ : FSAM_READ_WRITE;
    bool storage_res = storage_file_open(file, path, access_mode, FSOM_OPEN_EXISTING);

    return storage_res ? file : NULL;
}

static lv_fs_res_t gui_lvgl_fs_close(lv_fs_drv_t* drv, void* file_p) {
    UNUSED(drv);

    bool storage_res = storage_file_close(file_p);
    storage_file_free(file_p);

    return gui_lvgl_fs_convert_res(storage_res);
}

static lv_fs_res_t
    gui_lvgl_fs_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
    UNUSED(drv);

    *br = storage_file_read(file_p, buf, btr);

    return LV_FS_RES_OK;
}

static lv_fs_res_t
    gui_lvgl_fs_write(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw) {
    UNUSED(drv);

    *bw = storage_file_write(file_p, buf, btw);

    return LV_FS_RES_OK;
}

static lv_fs_res_t
    gui_lvgl_fs_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence) {
    UNUSED(drv);

    uint32_t seek_position = 0;
    uint32_t current_position = storage_file_tell(file_p);
    uint32_t size = storage_file_size(file_p);

    if(whence == LV_FS_SEEK_SET) {
        seek_position = current_position + pos;
    } else if(whence == LV_FS_SEEK_CUR) {
        seek_position = pos;
    } else if(whence == LV_FS_SEEK_END) {
        if(pos > size) {
            seek_position = 0;
        } else {
            seek_position = size - pos;
        }
    }
    bool storage_res = storage_file_seek(file_p, seek_position, true);

    return gui_lvgl_fs_convert_res(storage_res);
}

static lv_fs_res_t gui_lvgl_fs_tell(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p) {
    UNUSED(drv);

    *pos_p = storage_file_tell(file_p);

    return LV_FS_RES_OK;
}

static void* gui_lvgl_fs_dir_open(lv_fs_drv_t* drv, const char* path) {
    Storage* storage = drv->user_data;

    File* file = storage_file_alloc(storage);
    bool storage_res = storage_dir_open(file, path);

    return (storage_res == true) ? file : NULL;
}

static lv_fs_res_t
    gui_lvgl_fs_dir_read(lv_fs_drv_t* drv, void* rddir_p, char* fn, uint32_t fn_len) {
    UNUSED(drv);

    bool storage_res = storage_dir_read(rddir_p, NULL, fn, fn_len);

    return gui_lvgl_fs_convert_res(storage_res);
}

static lv_fs_res_t gui_lvgl_fs_dir_close(lv_fs_drv_t* drv, void* rddir_p) {
    UNUSED(drv);

    bool storage_res = storage_dir_close(rddir_p);
    storage_file_free(rddir_p);

    return gui_lvgl_fs_convert_res(storage_res);
}

static lv_fs_drv_t gui_lvgl_fs_driver = {
    .letter = 'C',
    .cache_size = 0,
    .ready_cb = NULL,
    .open_cb = gui_lvgl_fs_open,
    .close_cb = gui_lvgl_fs_close,
    .read_cb = gui_lvgl_fs_read,
    .write_cb = gui_lvgl_fs_write,
    .seek_cb = gui_lvgl_fs_seek,
    .tell_cb = gui_lvgl_fs_tell,
    .dir_open_cb = gui_lvgl_fs_dir_open,
    .dir_read_cb = gui_lvgl_fs_dir_read,
    .dir_close_cb = gui_lvgl_fs_dir_close,
};

void gui_lvgl_fs_init(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    gui_lvgl_fs_driver.user_data = storage;
    furi_record_close(RECORD_STORAGE);

    lv_fs_drv_register(&gui_lvgl_fs_driver);
}
