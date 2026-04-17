def skip_target_calc(method):
    def wrapper(self, model):
        # check if skip_target_calculation exists
        if not hasattr(model.profile, 'skip_target_calculation'):
            raise AttributeError("Expected attribute 'skip_target_calculation' not found in model.profile")

        # logic to skip target calculation
        if model.profile.skip_target_calculation:
            return
        return method(self, model)

    return wrapper