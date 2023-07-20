<script>
    import {Distributor, DistributorList} from "./Distributor"

    $: curDistributorId = 0;
    $: console.log(curDistributorId);
    

    let distributorList = new DistributorList();

    let distributor1 = new Distributor(0x0001, 0x0000000F, 1, 0, 127, 1, false, false, false)
    let distributor2 = new Distributor(0x0002, 0x000000F0, 1, 0, 127, 1, false, false, false)
    let distributor3 = new Distributor(0x0004, 0x00000F00, 1, 0, 127, 1, false, false, false)
    let distributor4 = new Distributor(0x0008, 0x0000F000, 1, 0, 127, 1, false, false, false)

    distributorList.append(distributor1);
    distributorList.append(distributor2);
    distributorList.append(distributor3);
    distributorList.append(distributor4);

</script>

<div class="div-outline h-full">

    <p class= 'text-center font-bold'>Distributor List</p>
    <div>
        {#each distributorList as distributor}
            <div class='select-none cursor-pointer' on:click={() => curDistributorId = distributor.getId()}>
                <span>{distributor.getId()}</span>
            </div>
        {/each}

    </div>

</div>