import logging

class mcu_fw_version_request:
    def __init__(self, config):
        logging.info(">>> mcu_fw_version_request __init__ called")

        printer = config.get_printer()
        self.printer = printer
        gcode = printer.lookup_object("gcode")

        mcu = self.printer.lookup_object('mcu')
        self.oid = mcu.create_oid()
        self.mcu = mcu
        gcode.register_command("MCUFWREAD", self.cmd_query_mcufw_r)

    def cmd_query_mcufw_r(self, gcmd):
        if not hasattr(self, '_query_cmd'):
            self.mcu_flash_cmd_queue = self.mcu.alloc_command_queue()
            self._query_cmd = self.mcu.lookup_query_command(
                "query_fw_version_read oid=%u",
                "response_version_read version=%s",
                cq=self.mcu_flash_cmd_queue,
                is_async=True
            )
        resp = self._query_cmd.send([self.oid, 0])
        mcufw_value = resp.get('version')
        gcmd.respond_info("MCU_VER[%s]" % mcufw_value.decode())

def load_config(config):
    logging.info(">>> load_config in mcu_fw_version_request called")
    return mcu_fw_version_request(config)
