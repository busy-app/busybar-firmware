# Board

This repository contains configuration and metadata about Silicon Labs boards.

## Repository structure

```
component/                -- slcc files for all boards + bsp
inc/                      -- include path for bsp
src/                      -- source path for bsp
config/                   -- board-specific component configuration
  component/              -- slcc files for board-specific component config
  <board_number>/         -- configuration headers for specific board
```