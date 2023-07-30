<script lang="ts">
  import { onMount } from 'svelte';

  type Variant = 'primary'| 'secondary' | 'outlined';

  export let onClick = () => {};
  export let variant: Variant = 'primary'; 
  export let className: string = ''; 
  export { className as class };

</script>

<button class="button {`button-${variant}`} {className}" on:click={onClick}>
    <slot></slot>
</button>

<style>
  .button {
    @apply py-2 px-4 rounded-md font-semibold;
  }

  .button-primary {
    @apply bg-primary text-white;
  }

  .button-secondary {
    @apply bg-secondary text-white;
  }

  .button-outlined {
    @apply bg-transparent border border-blue-500 text-blue-500;
  }
  </style>