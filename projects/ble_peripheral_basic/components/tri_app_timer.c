#include "tri_app_timer.h"
#include "app_timer.h"

static void timers_init(void)
{
    // 初始化app定时器模块
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // TODO: 创建用户定时任务代码
}
