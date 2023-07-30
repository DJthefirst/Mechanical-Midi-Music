import { toasts } from "svelte-toasts";

export interface ToastData {
  title: string;
  description: string;
}

const defaultToastOptions = {
  duration: 3000,
  placement: 'top-right',
  type: 'info',
  theme: 'dark',
  onClick: () => {},
  onRemove: () => {},
};

export const successToast = (payload: ToastData) => {
  toasts.success({
    title: payload.title,
    description: payload.description,
  });
};

export const failureToast = (payload: ToastData) => {
  toasts.error({
    title: payload.title,
    description: payload.description,
  });
};
