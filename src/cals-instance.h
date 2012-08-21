#include "cals-typedef.h"
#include "cals-time.h"

int cals_instance_insert(int event_id, struct cals_time *st, struct cals_time *et, cal_sch_full_t *sch);
int cals_instance_delete(int event_id, struct cals_time *st);

