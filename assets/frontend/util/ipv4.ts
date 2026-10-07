export function isValidIpv4 (value?: string) {
  const octets = value?.split('.') ?? [];

  return octets.length === 4 && octets.every(octet => {
    if (!/^\d{1,3}$/.test(octet)) {
      return false;
    }

    return Number(octet) <= 255;
  });
}
