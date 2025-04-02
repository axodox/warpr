import { Routes } from '@angular/router';
import { StreamHostComponent } from 'warpr-lib';

export const routes: Routes = [{ path: '**', component: StreamHostComponent }];
