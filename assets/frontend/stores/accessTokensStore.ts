import { defineStore } from 'pinia';
import type { AccessToken } from '@busy-app/busy-lib';

export const useAccessTokensStore = defineStore('accessTokens', () => {
  const deviceStore = useDeviceStore();

  const tokens = ref<AccessToken[]>([]);

  async function fetchTokens () {
    try {
      const response = await deviceStore.busyBar.SettingsAccessTokensGet();
      tokens.value = response.tokens || [];
    } catch (error) {
      await handleHTTPError(error, 'Couldn\'t get access tokens', true);
    }
  }

  async function revokeToken (shortId: string) {
    try {
      await deviceStore.busyBar.SettingsAccessTokenRevoke({ short_id: shortId });
      tokens.value = tokens.value.filter(token => token.short_id !== shortId);
      return true;
    } catch (error) {
      await handleHTTPError(error, 'Couldn\'t delete token');
      return false;
    }
  }

  return {
    tokens,

    fetchTokens,
    revokeToken
  };
});
