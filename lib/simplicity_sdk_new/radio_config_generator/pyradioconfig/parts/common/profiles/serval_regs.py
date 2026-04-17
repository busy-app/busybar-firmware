from pyradioconfig.parts.common.profiles.margay_regs import build_modem_regs_margay


def build_modem_regs_serval(model,profile):
    # Serval inherits from Margay
    build_modem_regs_margay(model, profile)
