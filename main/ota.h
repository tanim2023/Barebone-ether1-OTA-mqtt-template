#ifndef OTA_H
#define OTA_H

// Initialize OTA related components
void ota_init(void);

// OTA task function
void http_ota_task(void *pvParameter);

// Check if OTA needs verification or rollback
void ota_check_status(void);

#endif // OTA_H