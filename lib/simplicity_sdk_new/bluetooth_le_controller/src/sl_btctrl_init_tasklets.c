#include "sl_btctrl_linklayer.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include <sl_component_catalog.h>
#endif

void sl_btctrl_init_tasklets(void)
{
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT) \
  || defined(SLI_LL_SEQUENCER)
  sl_btctrl_init_adv_tasklet();
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CIS_PRESENT) \
  || defined(SLI_LL_SEQUENCER)
  sl_btctrl_init_iso_conn_tasklet();
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT) \
  || defined(SLI_LL_SEQUENCER)
  sl_btctrl_init_conn_tasklet();
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_TEST_PRESENT) \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_DTM_PRESENT) \
  || defined(SLI_LL_SEQUENCER)
  sl_btctrl_init_dtm_tasklet();
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT) \
  || defined(SLI_LL_SEQUENCER)
  //sl_btctrl_init_scan_tasklet();
#endif
}
