#include "sched.h"
#include "command.h"
#include "fw_version.h"

static struct task_wake wake_fw_version;

void command_fw_version_read(uint32_t *args)
{
    sched_wake_task(&wake_fw_version);
}
DECL_COMMAND(command_fw_version_read, "query_fw_version_read oid=%u");

void fw_version_read_flag(void)
{
    if (!sched_check_wake(&wake_fw_version))
        return;
    sendf("response_version_read version=%s", FW_VERSION);

}
DECL_TASK(fw_version_read_flag);
