<template>
  <div class="w-screen min-h-screen flex flex-row bg-white dark:bg-[#121212]">
    <UContainer class="w-full px-0 sm:px-0 lg:px-0">
      <DashboardLayoutSidebar class="hidden lg:flex" />
      <!-- sidebar width + 1px -->
      <div class="w-full lg:pl-[437px]">
        <div class="w-full p-8 min-h-screen flex flex-col items-start bg-neutral-50 dark:bg-neutral-900 ring-1 ring-gray-100 dark:ring-gray-800">
          <slot />
        </div>
      </div>
    </UContainer>
  </div>
</template>
