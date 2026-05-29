import logging

class mcu_flash_request:
    def __init__(self, config):
        logging.info(">>> mcu_flash_request __init__ called")

        printer = config.get_printer()
        self.printer = printer
        gcode = printer.lookup_object("gcode")

        mcu = self.printer.lookup_object('mcu')
        self.oid = mcu.create_oid()
        self.mcu = mcu
        gcode.register_command("MFLASHWRITE", self.cmd_query_flash_w)
        gcode.register_command("MFLASHREAD", self.cmd_query_flash_r)

    def cmd_query_flash_w(self, gcmd):
        self.flag = gcmd.get_int("FLAG", 0)
        flag_str = gcmd.get_command().strip().split()[1:]
        # 공백으로 분리하여 2번째 요소 이후 가져옴

        self.query_cmd = self.mcu.lookup_command(
            "query_flash_write oid=%u flag=%u"
        )
        self.query_cmd.send([self.oid, self.flag])

    def cmd_query_flash_r(self, gcmd):
        if not hasattr(self, '_query_cmd'):
            self.mcu_flash_cmd_queue = self.mcu.alloc_command_queue()
            self._query_cmd = self.mcu.lookup_query_command(
                "query_flash_read oid=%u",
                "response_flash_read flag=%u",
                cq=self.mcu_flash_cmd_queue,
                is_async=True
            )
        resp = self._query_cmd.send([self.oid, 0])
        flash_value = resp.get('flag')
        gcmd.respond_info("Flash value[%d]" % flash_value)

def load_config(config):
    logging.info(">>> load_config in mcu_flash_request called")
    return mcu_flash_request(config)
