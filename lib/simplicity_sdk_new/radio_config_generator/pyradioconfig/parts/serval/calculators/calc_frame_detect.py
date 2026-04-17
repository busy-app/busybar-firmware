from pyradioconfig.parts.margay.calculators.calc_frame_detect import CALC_Frame_Detect_Margay


class calc_frame_detect_serval(CALC_Frame_Detect_Margay):

    def calc_timthresh_value(self, model):
        mod_type = model.vars.modulation_type.value

        if mod_type == model.vars.modulation_type.var_enum.ASK:
            timthresh = 0
            model.vars.timing_detection_threshold.value = timthresh
        else:
            super().calc_timthresh_value(model)

    def calc_addtimseq_val(self, model):
        mod_format = model.vars.modulation_type.value

        if mod_format == model.vars.modulation_type.var_enum.ASK:
            # Always use 1 timing window for ASK
            addtimseq = 0
            model.vars.number_of_timing_windows.value = int(addtimseq) + 1
        else:
            super().calc_addtimseq_val(model)
