<script lang="ts">
    // Import the necessary Svelte modules
    import { SvelteComponent } from 'svelte';
	import Grid from './Grid.svelte';
    export let className: string = '';  
    export { className as class }
   </script>
   
   <div 
       class={`col-span-full md:col-span-6 ${ className }`}
   >
        <Grid>
            <slot />
        </Grid>
   </div>