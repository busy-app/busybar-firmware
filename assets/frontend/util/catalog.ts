// Public apps catalog (cloud.busy.app). The catalog API does not send CORS headers, so the
// browser cannot call it cross-origin from the device web interface. The dev server proxies it at
// /api/catalog/v1 (see server/api/catalog/v1/applications.get.ts); a production build talks to the
// API directly and the page reports the error when it is not reachable.
import type { AppQueueRequestOperation } from '@busy-app/busy-lib';

export interface CatalogApplicationBuild {
  id: string;
  sha256: string;
  size: number;
  download_uri: string;
}

export interface CatalogApplicationVersion {
  id: string;
  status: string;
  name: string;
  version: string;
  api_version: string;
  short_description?: string | null;
  current_build?: CatalogApplicationBuild | null;
  icon_uri?: string | null;
  icon_back_uri?: string | null;
  published_at?: number;
}

export interface CatalogApplication {
  id: string;
  alias: string;
  author?: string | null;
  category_id?: string;
  downloads?: number;
  is_official?: boolean;
  current_version?: CatalogApplicationVersion | null;
}

export interface CatalogApplicationsResponse {
  items: CatalogApplication[];
  total: number;
  limit: number;
  offset: number;
  next?: string | null;
  previous?: string | null;
}

export interface CatalogQuery {
  categoryId?: string;
  limit?: number;
  offset?: number;
}

export const CATALOG_PAGE_LIMIT = 24;

/** App id used by the queue operations: the catalog aliases match the ids in app manifests. */
export function catalogAppId (app: CatalogApplication): string {
  return app.alias;
}

/** Catalog applications that can actually be installed: the version is ready and has a package. */
export function isInstallable (app: CatalogApplication): boolean {
  return Boolean(app.current_version?.current_build?.download_uri);
}

export function catalogInstallOperation (app: CatalogApplication): AppQueueRequestOperation | null {
  const build = app.current_version?.current_build;
  if (!build?.download_uri) {
    return null;
  }

  return {
    type: 'install',
    url: build.download_uri,
    version: app.current_version?.version
  };
}

export function catalogIconUri (app: CatalogApplication | undefined): string | undefined {
  return app?.current_version?.icon_uri ?? undefined;
}

export function catalogTitle (app: CatalogApplication | undefined): string | undefined {
  return app?.current_version?.name ?? undefined;
}

export async function fetchCatalogApplications (query: CatalogQuery = {}): Promise<CatalogApplicationsResponse> {
  const baseURL = useRuntimeConfig().public.appsCatalogApiUrl;
  const { limit = CATALOG_PAGE_LIMIT, offset = 0 } = query;

  return await $fetch<CatalogApplicationsResponse>('/applications', {
    baseURL,
    query: {
      limit,
      offset,
      sort_by: 'recommended'
    }
  });
}
