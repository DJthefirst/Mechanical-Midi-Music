const daisyui = require("daisyui");
const typography = require("@tailwindcss/typography");

/** @type {import('tailwindcss').Config}*/
const config = {
  content: ["./src/**/*.{html,js,svelte,ts}"],
  mode: 'jit',
  theme: {
    extend: {
      colors: {
        gray: {
        'light': '#36393e',
        'dark': '#1e2124',
        },
        btn: {
          'green': {
            'default': '#06c258',
            'hover': '#06a94d',
            'press': '#06a94d',
            'disable': '#059142',
          },
          'red': {
            'default': '#ee5555',
            'hover': '#dd2211',
            'press': '#ca1111',
            'disable': '#b20000',
          }
          
        }
      }
    },
  },

  plugins: [typography, daisyui]
};

module.exports = config;