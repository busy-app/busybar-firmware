#pragma once

typedef enum {
    Ns4168Hpf_28Hz = (1U), /*!< 28Hz High Pass Filter */
    Ns4168Hpf_40Hz = (3U), /*!< 40Hz High Pass Filter */
    Ns4168Hpf_65Hz = (4U), /*!< 65Hz High Pass Filter */
    Ns4168Hpf_120Hz = (5U), /*!< 120Hz High Pass Filter */
    Ns4168Hpf_240Hz = (6U), /*!< 240Hz High Pass Filter */
    Ns4168Hpf_458Hz = (7U), /*!< 458Hz High Pass Filter */
    Ns4168Hpf_910Hz = (8U), /*!< 910Hz High Pass Filter */
    Ns4168Hpf_1820Hz = (9U), /*!< 1820Hz High Pass Filter */
    Ns4168Hpf_3500Hz = (10U), /*!< 3500Hz High Pass Filter */
    Ns4168Hpf_6600Hz = (11U), /*!< 6600Hz High Pass Filter */
    Ns4168Hpf_20Hz = (12U), /*!< 20Hz High Pass Filter */
} Ns4168Hpf;

#ifdef __cplusplus
extern "C" {
#endif

void ns4168_init(void);
void ns4168_deinit(void);
void ns4168_power_on(Ns4168Hpf hpf);
void ns4168_power_off(void);

#ifdef __cplusplus
}
#endif
