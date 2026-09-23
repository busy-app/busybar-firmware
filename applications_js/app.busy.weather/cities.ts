export type City = {
  name: string;
  lat: number;
  lon: number;
  tzName: string;
};

export const CITIES: City[] = [
  { name: "Amsterdam", lat: 52.37, lon: 4.9, tzName: "Europe/Amsterdam" },
  { name: "Chicago", lat: 41.88, lon: -87.63, tzName: "America/Chicago" },
  { name: "London", lat: 51.51, lon: -0.13, tzName: "Europe/London" },
  { name: "New York", lat: 40.71, lon: -74.01, tzName: "America/New_York" },
  { name: "Paris", lat: 48.86, lon: 2.35, tzName: "Europe/Paris" },
  { name: "Toronto", lat: 43.65, lon: -79.38, tzName: "America/Toronto" },
];

/** WMO weather code → icon name; missing conditions fall back to the closest. */
export function wmoToIcon(wmo: number): string {
  switch (wmo) {
    case 0:
      return "ic_clear";
    case 1:
      return "ic_pcloudy";
    case 2:
      return "ic_mcloudy";
    case 3:
      return "ic_cloudy";
    case 45:
    case 48:
      return "ic_foggy";
    case 51:
    case 53:
    case 56:
    case 61:
    case 66:
      return "ic_lightrain";
    case 55:
    case 57:
    case 63:
    case 65:
    case 67:
    case 82:
      return "ic_rain";
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
      return "ic_snow";
    case 80:
      return "ic_oshower";
    case 81:
      return "ic_ishower";
    case 95:
    case 96:
    case 99:
      return "ic_rain"; // no dedicated thunderstorm icon — closest match
    default:
      return "ic_cloudy";
  }
}

/** WMO weather code → animation name; missing conditions fall back likewise. */
export function wmoToAnim(wmo: number): string {
  switch (wmo) {
    case 0:
      return "w_clear";
    case 1:
      return "w_pcloudy";
    case 2:
      return "w_mcloudy";
    case 3:
      return "w_cloudy";
    case 45:
    case 48:
      return "w_foggy";
    case 51:
    case 53:
    case 56:
    case 61:
    case 66:
      return "w_lightrain";
    case 55:
    case 57:
    case 63:
    case 65:
    case 67:
    case 82:
    case 95:
    case 96:
    case 99:
      return "w_rain";
    case 80:
      return "w_oshower";
    case 81:
      return "w_ishower";
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
      return "w_rain"; // no dedicated snow animation — closest match
    default:
      return "w_cloudy";
  }
}
