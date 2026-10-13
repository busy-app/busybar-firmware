export const PASSWORD_MIN_LENGTH = 4;
export const PASSWORD_MAX_LENGTH = 32;

export const PASSWORD_RULES_ATTR = `allowed: upper, lower, digit; minlength: ${PASSWORD_MIN_LENGTH}; maxlength: ${PASSWORD_MAX_LENGTH};`;

export const PASSWORD_MASK = '*'.repeat(PASSWORD_MAX_LENGTH);
