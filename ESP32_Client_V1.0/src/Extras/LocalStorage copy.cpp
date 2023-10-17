// //TODO: Add Error handling

// #include <iostream>
// #include <string>
// #include "LocalStorage.h"
// #include "Distributor.h"

// esp_err_t err;
// nvs_handle_t handle;




// LocalStorage::LocalStorage(){
//     // Initialize NVS
//     err = nvs_flash_init();
//     if (err != ESP_OK) Serial.printf("Error (%s) Init NVS!\n", esp_err_to_name(err));
//     if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         // NVS partition was truncated and needs to be erased
//         // Retry nvs_flash_init
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         err = nvs_flash_init();
//         if (err != ESP_OK) Serial.printf("Error (%s) Init NVS!\n", esp_err_to_name(err));
//     }
//     ESP_ERROR_CHECK( err );
// }

// esp_err_t save_restart_counter(void)
// {
//     uint8_t restart_counter = 0; // value will default to 0, if not set yet in NVS
//     err = nvs_get_u8(handle, "restart_conter", &restart_counter);
//     if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

//     // Write
//     restart_counter++;
//     err = nvs_set_i8(handle, "restart_conter", restart_counter);
//     if (err != ESP_OK) return err;

//     // Commit
//     err = nvs_commit(handle);
//     if (err != ESP_OK) return err;

//     // Close
//     nvs_close(handle);
//     return ESP_OK;
// }

// /* Save new run time value in NVS
//    by first reading a table of previously saved values
//    and then adding the new value at the end of the table.
//    Return an error if anything goes wrong
//    during this process.
//  */
// esp_err_t save_run_str(uint8_t* data, uint8_t arrayLength)
// {
//     size_t arraySize = sizeof(uint8_t)*arrayLength;

//     err = nvs_set_blob(handle, "run_str", data, arrayLength);
//     if (err != ESP_OK) return err;

//     // Commit
//     err = nvs_commit(handle);
//     if (err != ESP_OK) return err;

//     // Close
//     nvs_close(handle);
//     return ESP_OK;
// }




// /* Read from NVS and print restart counter
//    and the table with run times.
//    Return an error if anything goes wrong
//    during this process.
//  */
// esp_err_t print_what_saved(uint8_t* result, uint8_t arrayLength)
// {
//     size_t arraySize = sizeof(uint8_t)*arrayLength;
    
//     // Valid Data
//     uint8_t tempResult[arrayLength];
//     err = nvs_get_blob(handle, "run_str", tempResult, &arraySize);
//     if (err != ESP_OK) return err;
//     std::copy(tempResult, tempResult+arraySize, result);
//     nvs_close(handle);
//     return ESP_OK; //Colse Handle ouside of this function
// }

// void LocalStorage::Test(){

//     // Open
//     uint8_t result[20];
//     OpenNvs();
//     err = print_what_saved(result, 20);
//     if (err != ESP_OK) printf("Error (%s) reading data from NVS!\n", esp_err_to_name(err));
//     else Serial.println((char*)result);


//     err = save_restart_counter();
//     if (err != ESP_OK) printf("Error (%s) saving restart counter to NVS!\n", esp_err_to_name(err));

//     OpenNvs();
//     uint8_t data[20]; 
//     for(int i=0; i<20; i++){
//         data[i] = (uint8_t)Device::Name.c_str()[i];
//     }
//     OpenNvs();
//     err = save_run_str(data, 20);
//     if (err != ESP_OK) printf("Error (%s) saving run time blob to NVS!\n", esp_err_to_name(err));
// }