<script>
    import Player from './Player/Player.svelte';
    import DeviceManager from './Devices/DeviceManager.svelte';
    import DeviceList from './Devices/DeviceList.svelte';
    import DistributorManager from './Distributors/DistributorManager.svelte';
    import DistributorList from './Distributors/DistributorList.svelte';
</script>

<div class= "grid grid-cols-1 md:grid-cols-5 xl:grid-cols-7
grid-rows-1 md:grid-rows-2">
<div class= "div-grid md:row-span-2 md:h-screen md:col-span-3 xl:col-span-3">

    <div>
        <Player />
    </div>
    <div>
        <DeviceManager />
    </div>
    <div>
        <DistributorManager /> 
    </div>
    
</div>
<div class= "div-grid md:col-start-4 xl:col-start-4 md:col-span-2
lg:row-start-1 row-span-1 xl:row-auto xl:h-screen ">

    <DeviceList />
</div>
<div class= "div-grid md:col-start-4 xl:col-start-6 md:col-span-2
lg:row-start-2 row-span-1 xl:row-auto xl:h-screen">

    <DistributorList />
</div>
</div>