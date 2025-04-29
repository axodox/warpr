import { Routes } from '@angular/router';
import { StreamHostComponent } from '../../../warpr-lib/src/public-api';

export const routes: Routes = [{ path: '**', component: StreamHostComponent }];
