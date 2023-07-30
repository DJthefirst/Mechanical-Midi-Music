<script lang="ts">
  import { onMount, createEventDispatcher } from 'svelte';

  interface UploadFilesEvent {
    upload: { data: FileList }
  }
  const dispatch = createEventDispatcher<UploadFilesEvent>();


  let isDragging = false;

  const handleDragEnter = (event: DragEvent) => {
    event.preventDefault();
    isDragging = true;
  };


  const handleDragLeave = () => {
    isDragging = false;
  };


  const handleDrop = (event: DragEvent) => {
    event.preventDefault();
    isDragging = false;
    const files = event.dataTransfer?.files;
    if (files && files.length > 0) {
      console.log(files);
      
      dispatch('upload', {
        data: files
      });
    }
  };
</script>

<div
  class:drop-area={true}
  class:drag-over={isDragging}
  on:dragenter={handleDragEnter}
  on:dragleave={handleDragLeave}
  on:dragover={handleDragEnter}
  on:drop={handleDrop}
>
  {#if isDragging}
    <p>Drop your files here</p>
  {:else}
    <p>Drag and drop files here</p>
  {/if}
</div>

<style>
     .drop-area {
      @apply p-6 text-3xl font-bold border-2 border-dashed rounded-lg border-blue-500 col-span-10 h-[40vh] bg-secondary;
      transition: border-color 0.3s ease;
      grid-column-start: 2;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .drag-over {
      @apply border-blue-700;
    } 
  </style>