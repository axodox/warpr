import { ComponentFixture, TestBed } from '@angular/core/testing';

import { StreamHostComponent } from './stream-host.component';

describe('StreamHostComponent', () => {
  let component: StreamHostComponent;
  let fixture: ComponentFixture<StreamHostComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [StreamHostComponent]
    })
    .compileComponents();
    
    fixture = TestBed.createComponent(StreamHostComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
