// The apps catalog API (cloud.busy.app) sends no CORS headers, so it cannot be called directly from
// the browser. This dev-only route proxies the catalog for the local web interface dev server.
// A production bundle talks to the catalog directly (NUXT_PUBLIC_APPS_CATALOG_API_URL) and reports the
// error when the browser blocks the response.
const CATALOG_API_ORIGIN = 'https://cloud.busy.app';
const CATALOG_API_PATH = '/api/catalog/v1/applications';

export default defineEventHandler(async (event): Promise<unknown> => {
  const query = getQuery(event);

  try {
    return await $fetch<Record<string, unknown>>(CATALOG_API_PATH, {
      baseURL: CATALOG_API_ORIGIN,
      query
    });
  } catch (error) {
    const status = (error as { status?: number }).status ?? 502;
    throw createError({
      statusCode: status,
      statusMessage: 'Apps catalog request failed',
      data: { error: error instanceof Error ? error.message : 'Apps catalog request failed' }
    });
  }
});
