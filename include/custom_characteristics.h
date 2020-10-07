#ifndef CUSTOM_CHARACTERISTICS_H
#define CUSTOM_CHARACTERISTICS_H

#define HOMEKIT_CHARACTERISTIC_DEW_POINT "095c46e2-278e-4e3c-b9e7-364622a0f501"
#define HOMEKIT_DECLARE_CHARACTERISTIC_DEW_POINT(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_DEW_POINT, \
    .description = "Dew Point", \
    .format = homekit_format_float, \
    .unit = homekit_unit_celsius, \
    .permissions = homekit_permissions_paired_read \
                | homekit_permissions_notify, \
    .min_value = (float[]) {-50}, \
    .max_value = (float[]) {50}, \
    .min_step = (float[]) {0.1}, \
    .value = HOMEKIT_FLOAT_(_value), \
    ##__VA_ARGS__

#endif
