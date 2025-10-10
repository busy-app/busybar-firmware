#!/bin/bash

set -e

matter_dir="$HOME/Files/Devel/connectedhomeip"
matter_cert_dir="$matter_dir/credentials/test"
matter_paa_dir="$matter_cert_dir/attestation"
matter_cd_dir="$matter_cert_dir/certification-declaration"

vendor_id="158A"
product_id="0001"
device_type_id="10A" # On/Off Pluggable Unit

pai_key_file="test-PAI-${vendor_id}-key.pem"
pai_cert_file="test-PAI-${vendor_id}-cert.pem"

dac_key_file="test-DAC-${vendor_id}-${product_id}-key.pem"
dac_cert_file="test-DAC-${vendor_id}-${product_id}-cert.pem"

cd_file="test-CD-${vendor_id}-${product_id}.der"

valid_from="$(date +%Y-%m-%d) 00:00:00"
valid_lifetime="4294967295"

# Remove old files

rm -f \
  "$pai_key_file" \
  "$pai_cert_file" \
  "$dac_key_file" \
  "$dac_cert_file" \
  "$cd_file"

# Generate PAI

chip-cert gen-att-cert --type i \
  --subject-cn "Matter Test PAI ${vendor_id}" \
  --subject-vid "${vendor_id}" \
  --valid-from "$valid_from" \
  --lifetime "$valid_lifetime" \
  --ca-key "$matter_paa_dir/Chip-Test-PAA-NoVID-Key.pem" \
  --ca-cert "$matter_paa_dir/Chip-Test-PAA-NoVID-Cert.pem" \
  --out-key "$pai_key_file" \
  --out "$pai_cert_file"

# Generate DAC

chip-cert gen-att-cert --type d \
  --subject-cn "Matter Test DAC ${vendor_id}/${product_id}" \
  --subject-vid "${vendor_id}" \
  --subject-pid "${product_id}" \
  --valid-from "$valid_from" \
  --lifetime "$valid_lifetime" \
  --ca-key "$pai_key_file" \
  --ca-cert "$pai_cert_file" \
  --out-key "$dac_key_file" \
  --out "$dac_cert_file"

# Verify chain

chip-cert validate-att-cert \
  --dac "$dac_cert_file" \
  --pai "$pai_cert_file" \
  --paa "$matter_paa_dir/Chip-Test-PAA-NoVID-Cert.pem"

# Generate CD

chip-cert gen-cd \
  --key "$matter_cd_dir/Chip-Test-CD-Signing-Key.pem" \
  --cert "$matter_cd_dir/Chip-Test-CD-Signing-Cert.pem" \
  --out "$cd_file" \
  --format-version "1" \
  --vendor-id "${vendor_id}" \
  --product-id "${product_id}" \
  --device-type-id "${device_type_id}" \
  --certificate-id "CSA00000SWC00000-00" \
  --security-level "0" \
  --security-info "0" \
  --version-number "1" \
  --certification-type "0"


