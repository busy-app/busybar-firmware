# Intro

Special firmware configuration for factory testing of the OLED display.

# Build locally

    ./fbt --options=`realpath fbt_factory_oled_opts.py`

# Build in CI

Changed `.github/workflows/build.yml` to include the following extra_fbt_args in the build step:

          extra_fbt_args: "--options=fbt_factory_oled_opts.py"
