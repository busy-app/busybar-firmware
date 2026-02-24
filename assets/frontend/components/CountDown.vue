<template>
  <span>{{ formatDuration(remainingTime) }}</span>
</template>

<script setup lang="ts">
import { clearInterval, setInterval } from 'worker-timers';

const props = defineProps<{
  ms: number;
}>();

const emit = defineEmits(['complete']);

const remainingTime = ref(0);
const timerInterval = ref(null as number | null);

const startTimer = () => {
  timerInterval.value = setInterval(() => {
    if (remainingTime.value > 0) {
      let currentTime = remainingTime.value;
      currentTime -= 1000;

      if (currentTime < 1000) {
        currentTime = 0;
      }

      remainingTime.value = currentTime;
    } else if (timerInterval.value) {
      clearInterval(timerInterval.value);
      remainingTime.value = 0;
      emit('complete');
    }
  }, 1000);
};

const formatDuration = (milliseconds: number) => {
  const totalSeconds = Math.floor(milliseconds / 1000);
  const seconds = totalSeconds % 60;
  const totalMinutes = Math.floor(totalSeconds / 60);
  const minutes = totalMinutes % 60;
  const hours = Math.floor(totalMinutes / 60);

  const pad = (number: number) => number.toString().padStart(2, '0');

  if (hours > 0) {
    return `${pad(hours)}:${pad(minutes)}:${pad(seconds)}`;
  } else {
    return `${pad(minutes)}:${pad(seconds)}`;
  }
};

onMounted(() => {
  remainingTime.value = props.ms;
  startTimer();
});

onBeforeUnmount(() => {
  if (timerInterval.value) {
    clearInterval(timerInterval.value);
  }
});
</script>
