# Wi-SUN - RCP Border Router

The Wi-SUN RCP Border Router application is a radio coprocessor implementation that has to be paired with a Linux host running the Wi-SUN stack upper layers (i.e., wsbrd).

## Getting Started

To get started with Wi-SUN and Simplicity Studio, see [Developing with Wi-SUN](https://docs.silabs.com/wisun/latest/wisun-start/).

The RCP term stands for "radio coprocessor", meaning the application runs on the EFR32 and requires an external Linux host. The Linux host has to run the [wsbrd daemon hosted on GitHub](https://github.com/SiliconLabs/wisun-br-linux). Using both the linux host and the EFR32 running the RCP image, the aim is to create a Wi-SUN border router that can scale to support large network deployments.

![Linux Border Router Architecture](readme_img1.png)

## Troubleshooting

Before programming the radio board mounted on the WSTK, ensure the power supply switch is in the AEM position (right side), as shown.

![Radio Board Power Supply Switch](readme_img0.png)

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun).
