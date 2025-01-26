// Number of ticks elapsed per millisecond (48,000 when using 48MHz Clock)
#define SYSTICK_ONE_MILLISECOND ((uint32_t)FUNCONF_SYSTEM_CORE_CLOCK / 1000)
// Number of ticks elapsed per microsecond (48 when using 48MHz Clock)
#define SYSTICK_ONE_MICROSECOND ((uint32_t)FUNCONF_SYSTEM_CORE_CLOCK / 1000000)

void systick_init();
