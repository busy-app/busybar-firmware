// @ts-check
import stylistic from '@stylistic/eslint-plugin';
import tailwindPlugin from 'eslint-plugin-tailwindcss';
import { withNuxt } from './.nuxt/eslint.config.mjs';

/** @type {import('eslint').ESLint.Plugin} */
const tailwind = /** @type {any} */ (tailwindPlugin);

export default withNuxt(
  {},

  /** @type {import('eslint').Linter.Config} */ (tailwindPlugin.configs.recommended),

  {
    ignores: ['**/.output', '**/.nitro', '**/.netlify', '**/.nuxt', '**/*.gen.*'],
    plugins: {
      '@stylistic': stylistic,
      'tailwindcss': tailwind
    },
    settings: {
      tailwindcss: {
        cssConfigPath: './assets/css/global.css'
      }
    },
    rules: {
      'curly': 'error',
      'eqeqeq': 'error',

      '@stylistic/array-bracket-spacing': ['error', 'never'],
      '@stylistic/arrow-parens': ['error', 'as-needed'],
      '@stylistic/arrow-spacing': 'error',
      '@stylistic/block-spacing': 'error',
      '@stylistic/brace-style': ['error', '1tbs'],
      '@stylistic/comma-dangle': ['error', 'never'],
      '@stylistic/comma-style': 'error',
      '@stylistic/comma-spacing': 'error',
      '@stylistic/computed-property-spacing': ['error', 'never'],
      '@stylistic/curly-newline': ['error', 'always'],
      '@stylistic/eol-last': 'error',
      '@stylistic/function-call-argument-newline': ['error', 'consistent'],
      '@stylistic/function-call-spacing': ['error', 'never'],
      '@stylistic/function-paren-newline': ['error', 'multiline'],
      '@stylistic/indent': ['error', 2],
      '@stylistic/implicit-arrow-linebreak': ['error', 'beside'],
      '@stylistic/indent-binary-ops': ['error', 2],
      '@stylistic/key-spacing': ['error', { beforeColon: false }],
      '@stylistic/keyword-spacing': ['error', { after: true, before: true }],
      '@stylistic/max-statements-per-line': ['error', { max: 1 }],
      '@stylistic/member-delimiter-style': 'error',
      '@stylistic/new-parens': 'error',
      '@stylistic/no-extra-semi': 'error',
      '@stylistic/no-mixed-operators': 'error',
      '@stylistic/no-mixed-spaces-and-tabs': 'error',
      '@stylistic/no-multi-spaces': 'error',
      '@stylistic/no-multiple-empty-lines': ['error', { max: 1 }],
      '@stylistic/no-tabs': 'error',
      '@stylistic/no-trailing-spaces': 'error',
      '@stylistic/no-unused-vars': 0,
      '@stylistic/no-whitespace-before-property': 'error',
      '@stylistic/operator-linebreak': [
        'error',
        'before',
        {
          overrides: {
            '=': 'after'
          }
        }
      ],
      '@stylistic/quotes': ['error', 'single'],
      '@stylistic/rest-spread-spacing': ['error', 'never'],
      '@stylistic/semi': ['error', 'always'],
      '@stylistic/semi-spacing': 'error',
      '@stylistic/semi-style': ['error', 'last'],
      '@stylistic/space-before-blocks': 'error',
      '@stylistic/space-before-function-paren': ['error', 'always'],
      '@stylistic/space-infix-ops': 'error',
      '@stylistic/space-in-parens': 'error',
      '@stylistic/space-unary-ops': 'error',
      '@stylistic/spaced-comment': ['error', 'always'],
      '@stylistic/switch-colon-spacing': 'error',
      '@stylistic/type-annotation-spacing': 'error',
      '@stylistic/type-generic-spacing': ['error'],
      '@stylistic/type-named-tuple-spacing': ['error'],

      'unicorn/consistent-function-scoping': 0,
      'unicorn/filename-case': 0,
      'unicorn/numeric-separators-style': 0,
      'unicorn/no-null': 0,

      'vue/multi-word-component-names': 'off',
      'vue/html-indent': ['error', 2, {
        attribute: 1,
        baseIndent: 1,
        closeBracket: 0,
        alignAttributesVertically: false
      }],
      'vue/html-closing-bracket-newline': ['error', {
        singleline: 'never',
        multiline: 'always'
      }],
      'vue/first-attribute-linebreak': ['error', {
        singleline: 'beside',
        multiline: 'below'
      }],
      'vue/max-attributes-per-line': ['error', {
        singleline: 1,
        multiline: 1
      }],
      'vue/no-multi-spaces': 'error',
      'vue/singleline-html-element-content-newline': 'off',
      'vue/prefer-use-template-ref': 'error',
      'vue/block-order': ['error', {
        order: ['template', 'script', 'style']
      }],

      'tailwindcss/classnames-order': 'error',
      'tailwindcss/no-custom-classname': ['error', {
        whitelist: ['rounded-group', 'color-picker-alpha-.+', 'fim', 'fim-focus-.+', 'fm-item']
      }],
      'tailwindcss/no-unnecessary-arbitrary-value': 'error',
      'tailwindcss/enforces-negative-arbitrary-values': 'off'
    }
  }
);
