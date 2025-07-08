// @ts-check
import stylisticTs from '@stylistic/eslint-plugin-ts';
import { withNuxt } from './.nuxt/eslint.config.mjs';

export default withNuxt(
  {},
  {
    plugins: {
      '@stylistic/ts': stylisticTs
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
      '@stylistic/no-mixed-operators': 'error',
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

      // https://eslint.style/packages/ts#rules
      '@stylistic/ts/block-spacing': 'error',
      '@stylistic/ts/brace-style': 'error',
      '@stylistic/ts/comma-dangle': ['error', 'never'],
      '@stylistic/ts/comma-spacing': 'error',
      '@stylistic/ts/function-call-spacing': ['error', 'never'],
      '@stylistic/ts/indent': 'off', // https://github.com/typescript-eslint/typescript-eslint/issues/1824
      '@stylistic/ts/key-spacing': ['error', { beforeColon: false }],
      '@stylistic/ts/keyword-spacing': ['error', { after: true, before: true }],
      '@stylistic/ts/member-delimiter-style': 'error',
      '@stylistic/ts/no-extra-semi': 'error',
      '@stylistic/ts/no-unused-vars': 0,
      '@stylistic/ts/quotes': ['error', 'single'],
      '@stylistic/ts/semi': 'error',
      '@stylistic/ts/space-before-blocks': 'error',
      '@stylistic/ts/space-before-function-paren': 'error',
      '@stylistic/ts/space-infix-ops': 'error',
      '@stylistic/ts/type-annotation-spacing': 'error',

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
