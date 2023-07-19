<script>
    import Player from './Player/Player.svelte';
    import DeviceManager from './DeviceManager.svelte';
    import DeviceList from './DeviceList.svelte';
    import DistributorManager from './DistributorManager.svelte';
    import DistributorList from './DistributorList.svelte';
</script>

<div class= "grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3
grid-rows-1 md:grid-rows-2">
<div class= "div-grid md:row-span-2 md:h-screen">

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
<div class= "div-grid md:col-start-2
md:row-start-1 row-span-1 xl:row-auto xl:h-screen ">

    <DeviceList />
</div>
<div class= "div-grid md:col-start-2 xl:col-start-3
md:row-start-2 row-span-1 xl:row-auto xl:h-screen">

    <DistributorList />
</div>
</div>