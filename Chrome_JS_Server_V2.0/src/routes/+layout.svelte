<script lang="ts">
	import '../app.css';
	import { dev } from '$app/environment';
	import { inject } from '@vercel/analytics';
</script>

<div class="flex flex-col">
	<!-- <div class='flex flex-row justify-between p-4 m-2  rounded-2xl outline outline-gray-light outline-4'>
		<span>GitHub</span>
		<span>Mechanical Midi Music</span>
		<span>Wiki</span>
	</div> -->
	<slot />
</div>
