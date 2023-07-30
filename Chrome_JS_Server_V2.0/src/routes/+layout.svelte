<script>
	import '../app.css';
	import { dev } from '$app/environment';
	import { inject } from '@vercel/analytics';
	import { toasts, ToastContainer, FlatToast, BootstrapToast }  from "svelte-toasts";


	inject({ mode: dev ? 'development' : 'production' });
</script>

<ToastContainer placement="top-right" let:data={data}>
    <FlatToast {data} /> 
</ToastContainer>
<slot />
