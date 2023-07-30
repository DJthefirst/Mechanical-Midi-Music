<script lang="ts">
 // Import the necessary Svelte modules
 import { SvelteComponent } from 'svelte';
 export let className: string = '';  
 export { className as class }
</script>

<div 
    class={`grid grid-cols-12 gap-4 justify-center items-center ${ className }`}
>
    <slot />
</div>