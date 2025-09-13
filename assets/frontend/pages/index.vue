<template>
  <div class="w-full flex flex-col gap-4">
    <div class="w-full flex justify-between items-center ring-1 ring-glass rounded-3xl p-6 bg-elevated/50">
      <div class="flex items-center gap-4">
        <div class="size-14 rounded-full bg-elevated p-[14px]">
          <UIcon
            name="i-tabler-wifi"
            class="size-7"
          />
        </div>

        <div>
          <div class="text-xl">Wi-Fi</div>
          <div class="text-muted">Disconnected</div>
        </div>
      </div>

      <div>
        <UButton
          label="Select network"
          :ui="{
            base: 'px-2.5 py-2 rounded-full'
          }"
        />
      </div>
    </div>

    <UCard
      :ui="{
        root: 'ring-1 ring-glass rounded-3xl bg-elevated/50',
        header: 'p-4 sm:p-6',
        body: 'p-4 sm:p-6',
        footer: 'p-4 sm:p-6'
      }"
    >
      <template #header>
        <div class="w-full flex justify-between items-center">
          <div class="flex items-center gap-4">
            <div class="size-14 rounded-full bg-elevated p-[14px]">
              <UIcon
                name="i-tabler-wifi"
                class="size-7"
              />
            </div>

            <div>
              <div class="text-xl">Wi-Fi</div>
              <div class="text-muted">Disconnected</div>
            </div>
          </div>

          <div>
            <UButton
              label="Select network"
              :ui="{
                base: 'px-2.5 py-2 rounded-full'
              }"
            />
          </div>
        </div>
      </template>
    </UCard>
  </div>
</template>

<script setup lang="ts">
useHead({
  title: 'BUSY Bar Virtual LAN',
  meta: [
    {
      name: 'description',
      content: 'Control your BUSY Bar in the browser'
    }
  ]
});
</script>
