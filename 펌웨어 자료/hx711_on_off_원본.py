#VERSION: 1.0
class HX711OnOff:
    def __init__(self, config):
        self.printer = config.get_printer()
        self.gcode = self.printer.lookup_object("gcode")

        try:
            self.hx71x = self.printer.lookup_object("load_cell")
            self.sensor = self.hx71x.sensor  # 내부 HX71xBase 객체
            self.mcu = self.sensor.mcu
            self.cmd_query = self.sensor.query_hx71x_cmd
            self.oid = self.sensor.oid
            self.gcode.respond_info(
                f"[HX711OnOff] Linked to HX71x (oid={self.oid})"
            )
        except Exception as e:
            self.gcode.respond_info(f"[HX711OnOff] Failed to link HX71x: {e}")
            raise config.error(
                "HX71x instance not found — check [hx71x] section is loaded"
            )

        self.sps = config.getint("sps", 15000)

        self.gcode.register_command(
            "TP",
            self.cmd_TP,
            desc="Start HX711 manually"
        )        

        self.printer.register_event_handler(
            "home_z_start",self._on_hx711_start
        )
        self.printer.register_event_handler(
            "homing_end", self._on_hx711_end
        )
        self.printer.register_event_handler(
            "bed_mesh_calibrate:start", self._on_hx711_start
        )
        self.printer.register_event_handler(
            "bed_mesh_calibrate:end", self._on_hx711_end
        )

        self.gcode.respond_info(
            f"[HX711OnOff] initialized (oid={self.oid}, sps={self.sps})"
        )

    def cmd_TP(self, gcmd):
        eventtime = self.printer.get_reactor().monotonic()
        self._on_hx711_start(eventtime)        

    def _on_hx711_start(self, eventtime, *args, **kwargs):
        self._send_query_start()
        self.gcode.respond_info("---HX711 started---")

    def _on_hx711_end(self, eventtime, *args, **kwargs):
        self._send_query_stop()
        self.gcode.respond_info("---HX711 stopped---")

    def _send_query_start(self):
        try:
            if self.cmd_query is None:
                self.cmd_query = self.mcu.lookup_command(
                    "query_hx71x oid=%c rest_ticks=%u", None
                )

            rest_ticks = self.mcu.seconds_to_clock(1.0 / (10.0 * self.sps))

            self.cmd_query.send([self.oid, int(rest_ticks)])

        except Exception as e:
            self.gcode.respond_info(f"[HX711] start failed: {e}")

    def _send_query_stop(self):
        try:
            if self.cmd_query is None:
                self.cmd_query = self.mcu.lookup_command(
                    "query_hx71x oid=%c rest_ticks=%u", None
                )

            self.cmd_query.send([self.oid, 0])
        except Exception as e:
            self.gcode.respond_info(f"[HX711] stop failed: {e}")


def load_config(config):
    return HX711OnOff(config)
load_config.requires_objects = ["load_cell"]
