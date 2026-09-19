import type { DecodedAnimation } from '@/util/anim2seq';

export interface AnimationTickerSubscriber {
  animation: DecodedAnimation;
  context: CanvasRenderingContext2D;
  loop: boolean;
  playing: boolean;
  frameIndex: number;
  elapsedInFrame: number;
}

const subscribers = new Set<AnimationTickerSubscriber>();
let frameHandle: number | null = null;
let lastTimestamp: number | null = null;

export function drawAnimationFrame (subscriber: AnimationTickerSubscriber, index: number) {
  const frame = subscriber.animation.frames[index];

  if (frame) {
    subscriber.context.putImageData(frame.imageData, 0, 0);
  }
}

function advance (subscriber: AnimationTickerSubscriber, deltaMs: number) {
  const frames = subscriber.animation.frames;

  if (frames.length < 2) {
    return;
  }

  const frameDuration = 1000 / Math.max(1, subscriber.animation.fps);
  let advanced = false;

  subscriber.elapsedInFrame += deltaMs;

  while (subscriber.elapsedInFrame >= frameDuration * frames[subscriber.frameIndex].duration) {
    subscriber.elapsedInFrame -= frameDuration * frames[subscriber.frameIndex].duration;

    if (subscriber.frameIndex + 1 >= frames.length) {
      if (!subscriber.loop) {
        subscriber.elapsedInFrame = 0;
        subscriber.playing = false;
        break;
      }

      subscriber.frameIndex = 0;
    } else {
      subscriber.frameIndex += 1;
    }

    advanced = true;
  }

  if (advanced) {
    drawAnimationFrame(subscriber, subscriber.frameIndex);
  }
}

function tick (timestamp: number) {
  frameHandle = null;

  const deltaMs = lastTimestamp === null ? 0 : timestamp - lastTimestamp;
  lastTimestamp = timestamp;

  let anyPlaying = false;

  subscribers.forEach(subscriber => {
    if (!subscriber.playing) {
      return;
    }

    advance(subscriber, deltaMs);
    anyPlaying = anyPlaying || subscriber.playing;
  });

  if (anyPlaying) {
    frameHandle = requestAnimationFrame(tick);
  } else {
    lastTimestamp = null;
  }
}

export function requestAnimationTick () {
  if (frameHandle !== null || typeof requestAnimationFrame === 'undefined') {
    return;
  }

  lastTimestamp = null;
  frameHandle = requestAnimationFrame(tick);
}

export function subscribeToAnimationTicker (subscriber: AnimationTickerSubscriber) {
  subscribers.add(subscriber);

  if (subscriber.playing) {
    requestAnimationTick();
  }

  return () => {
    subscribers.delete(subscriber);
  };
}
