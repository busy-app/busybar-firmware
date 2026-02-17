// @ts-check
import stylistic from '@stylistic/eslint-plugin';
import { withNuxt } from './.nuxt/eslint.config.mjs';

export default withNuxt(
  {},
  {
    plugins: {
      '@stylistic': stylistic
    },
    ignores: ['**/.output', '**/.nitro', '**/.netlify', '**/.nuxt', '**/*.gen.*'],
    rules: {
      'curly': 'error',
      'eqeqeq': 'error',

      // https://eslint.style/packages/default#rules
      '@stylistic/array-bracket-spacing': ['error', 'never'],
      '@stylistic/arrow-parens': ['error', 'as-needed'],
      '@stylistic/arrow-spacing': 'error',
      '@stylistic/comma-style': 'error',
      '@stylistic/computed-property-spacing': ['error', 'never'],
      '@stylistic/curly-newline': ['error', 'always'],
      '@stylistic/eol-last': 'error',
      '@stylistic/function-call-argument-newline': ['error', 'consistent'],
      '@stylistic/function-paren-newline': ['error', 'multiline'],
      '@stylistic/implicit-arrow-linebreak': ['error', 'beside'],
      '@stylistic/indent-binary-ops': ['error', 2],
      '@stylistic/max-statements-per-line': ['error', { max: 1 }],
      '@stylistic/new-parens': 'error',
      '@stylistic/no-mixed-operators': 'off',
      '@stylistic/no-mixed-spaces-and-tabs': 'error',
      '@stylistic/no-multi-spaces': 'error',
      '@stylistic/no-multiple-empty-lines': ['error', { max: 1 }],
      '@stylistic/no-tabs': 'error',
      '@stylistic/no-trailing-spaces': 'error',
      '@stylistic/no-whitespace-before-property': 'error',
      '@stylistic/operator-linebreak': ['error', 'before'],
      '@stylistic/rest-spread-spacing': ['error', 'never'],
      '@stylistic/semi-spacing': 'error',
      '@stylistic/semi-style': ['error', 'last'],
      '@stylistic/space-before-function-paren': ['error', 'always'],
      '@stylistic/space-in-parens': 'error',
      '@stylistic/space-unary-ops': 'error',
      '@stylistic/spaced-comment': ['error', 'always'],
      '@stylistic/switch-colon-spacing': 'error',
      '@stylistic/type-generic-spacing': ['error'],
      '@stylistic/type-named-tuple-spacing': ['error'],

      // ts
      '@stylistic/block-spacing': 'error',
      '@stylistic/brace-style': 'error',
      '@stylistic/comma-dangle': ['error', 'never'],
      '@stylistic/comma-spacing': 'error',
      '@stylistic/function-call-spacing': ['error', 'never'],
      '@stylistic/indent': ['error', 2], // careful, https://github.com/typescript-eslint/typescript-eslint/issues/1824
      '@stylistic/key-spacing': ['error', { beforeColon: false }],
      '@stylistic/keyword-spacing': ['error', { after: true, before: true }],
      '@stylistic/member-delimiter-style': 'error',
      '@stylistic/no-extra-semi': 'error',
      '@stylistic/no-unused-vars': 0,
      '@stylistic/quotes': ['error', 'single'],
      '@stylistic/semi': 'error',
      '@stylistic/space-before-blocks': 'error',
      '@stylistic/space-infix-ops': 'error',
      '@stylistic/type-annotation-spacing': 'error',

      'unicorn/consistent-function-scoping': 0,
      'unicorn/filename-case': 0,
      'unicorn/numeric-separators-style': 0,
      'unicorn/no-null': 0,

      // Vue specific
      'vue/no-multi-spaces': 'error',
      'vue/singleline-html-element-content-newline': 'off'
    }
  }
);
