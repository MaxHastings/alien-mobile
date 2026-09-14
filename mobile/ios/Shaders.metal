#include <metal_stdlib>

using namespace metal;

struct CameraUniforms {
    float centerX;
    float centerY;
    float zoom;
    float padding;
    float viewportWidth;
    float viewportHeight;
    float worldWidth;
    float worldHeight;
};

struct GpuCellState {
    float2 position;
    float2 velocity;
};

struct RenderConnection {
    uint cellA;
    uint cellB;
    float colorR;
    float colorG;
    float colorB;
    float colorA;
};

struct RenderCellMetadata {
    float radius;
    float alpha;
    float colorR;
    float colorG;
    float colorB;
    float padding0;
    float padding1;
    float padding2;
};

struct GpuNeighbor {
    uint neighborIndex;
    float restLength;
    float stiffness;
    uint padding;
};

struct GpuAngle {
    uint cellA, center, cellB, slot;
    float targetAngle, stiffness, padding0, padding1;
};

struct GpuAdjacency {
    uint offset;
    uint count;
    uint padding0;
    uint padding1;
};

struct GpuPhysicsParams {
    float dt;
    float drag;
    float connectionDamping;
    float repulsionRadius;
    float repulsionStrength;
    float worldMinX;
    float worldMinY;
    float worldMaxX;
    float worldMaxY;
    float boundaryRestitution;
    float cellRadius;
    uint cellCount;
    uint toroidal;
    uint gridX, gridY;
};

struct LineOutput {
    float4 position [[position]];
    float4 color;
    float side;
};

struct CellOutput {
    float4 position [[position]];
    float2 local;
    float kind;
    float2 thrust;
    float4 color;
};

float4 worldToClip(float2 worldPosition, constant CameraUniforms& camera)
{
    float minimumDimension = max(1.0f, min(camera.viewportWidth, camera.viewportHeight));
    float2 relative = worldPosition - float2(camera.centerX, camera.centerY);
    return float4(relative * camera.zoom * minimumDimension
        / float2(camera.viewportWidth, camera.viewportHeight), 0.0f, 1.0f);
}

float2 renderDelta(float2 d,constant CameraUniforms& c) {
    if(c.worldWidth>0) {float2 size=float2(c.worldWidth,c.worldHeight);d-=size*round(d/size);}
    return d;
}
float2 imageOffset(uint instance,constant CameraUniforms& c) {
    if(c.worldWidth<=0) return float2(0);
    return float2(int(instance%3)-1,int((instance/3)%3)-1)*float2(c.worldWidth,c.worldHeight);
}
vertex LineOutput lineVertex(
    uint vertexID [[vertex_id]],
    uint instanceID [[instance_id]],
    const device RenderConnection* connections [[buffer(0)]],
    const device GpuCellState* states [[buffer(2)]],
    constant CameraUniforms& camera [[buffer(1)]])
{
    RenderConnection connection = connections[camera.worldWidth>0 ? instanceID/9 : instanceID];
    constexpr float2 quad[6] = {float2(0,-1),float2(1,-1),float2(0,1),
        float2(0,1),float2(1,-1),float2(1,1)};
    float2 a = states[connection.cellA].position;
    float2 b = states[connection.cellB].position;
    float2 edge = renderDelta(b - a,camera);
    a=float2(camera.centerX,camera.centerY)+renderDelta(a-float2(camera.centerX,camera.centerY),camera)+imageOffset(instanceID,camera);
    b=a+edge;
    float2 normal = float2(-edge.y, edge.x) / max(length(edge), 0.00001f);
    LineOutput output;
    output.side = quad[vertexID].y;
    output.position = worldToClip(mix(a,b,quad[vertexID].x) + normal * output.side * 0.065f, camera);
    output.color = float4(connection.colorR, connection.colorG, connection.colorB, connection.colorA);
    return output;
}

fragment float4 lineFragment(LineOutput input [[stage_in]])
{
    float glow = exp(-input.side * input.side * 3.0f);
    return float4(input.color.rgb, input.color.a * glow);
}

vertex CellOutput cellVertex(
    uint vertexID [[vertex_id]],
    uint instanceID [[instance_id]],
    const device GpuCellState* states [[buffer(0)]],
    const device RenderCellMetadata* metadata [[buffer(2)]],
    constant CameraUniforms& camera [[buffer(1)]])
{
    constexpr float2 quad[6] = {
        float2(-1.0f, -1.0f),
        float2(1.0f, -1.0f),
        float2(-1.0f, 1.0f),
        float2(-1.0f, 1.0f),
        float2(1.0f, -1.0f),
        float2(1.0f, 1.0f),
    };
    uint index=camera.worldWidth>0 ? instanceID/9 : instanceID;
    GpuCellState state = states[index];
    state.position=float2(camera.centerX,camera.centerY)+renderDelta(state.position-float2(camera.centerX,camera.centerY),camera)+imageOffset(instanceID,camera);
    RenderCellMetadata instance = metadata[index];
    float2 local = quad[vertexID];
    CellOutput output;
    output.position = worldToClip(state.position + local * instance.radius, camera);
    output.local = local;
    output.kind = instance.padding0;
    output.thrust = float2(instance.padding1,instance.padding2);
    output.color = float4(instance.colorR, instance.colorG, instance.colorB, instance.alpha);
    return output;
}

fragment float4 cellFragment(CellOutput input [[stage_in]])
{
    float r = length(input.local);
    float aa = max(fwidth(r), 0.002f);
    if (r > 1.0f) discard_fragment();
    if (input.kind > 0.5f && input.kind < 1.5f) {
        // Field opacity follows the same smoothstep falloff as biology.
        float falloff = (1.0f - smoothstep(0.0f, 1.0f, r));
        float rim = (1.0f - smoothstep(0.008f, 0.008f + aa, abs(r - 0.97f))) * 0.25f;
        return float4(input.color.rgb, input.color.a * (falloff + rim));
    }
    if(input.kind>4.5f) {
        float ring=1-smoothstep(.018f,.018f+aa,abs(r-.82f));
        return float4(input.color.rgb,input.color.a*ring);
    }
    if(input.kind>3.5f) {
        return float4(input.color.rgb,input.color.a*exp(-r*r*7.0f));
    }
    if (input.kind > 1.5f) {
        float ring = 1.0f - smoothstep(0.055f, 0.055f + aa, abs(r - 0.68f));
        float mark;
        if (input.kind > 2.5f) {
            mark = 1.0f - smoothstep(0.045f, 0.045f + aa,
                min(abs(input.local.x - input.local.y), abs(input.local.x + input.local.y)));
        } else {
            mark = 1.0f - smoothstep(0.10f, 0.10f + aa, r);
        }
        mark *= 1.0f - smoothstep(0.32f, 0.36f, r);
        return float4(input.color.rgb, input.color.a * max(ring, max(mark, 0.15f * exp(-r*r*3.0f))));
    }
    float core = 1.0f - smoothstep(0.47f - aa, 0.47f + aa, r);
    float glow = exp(-r * r * 5.5f) * 0.48f;
    float nucleus = 1.0f - smoothstep(0.10f, 0.21f, r);
    // Lineage hue stays dominant; organs differ only in small inner marks.
    // The sign encodes the generic CellRole.  These are anatomy cues, not a
    // second palette: blue descendants still read as blue descendants.
    if(input.kind<-2.5f && input.kind>-3.5f) { // motor: directional core
        float magnitude=length(input.thrust);
        float2 forward=magnitude>0.00001f ? input.thrust/magnitude : float2(1,0);
        float axial=dot(input.local,forward), lateral=dot(input.local,float2(-forward.y,forward.x));
        nucleus=smoothstep(-.18f,.12f,axial)*(1-smoothstep(.46f,.62f,axial))*exp(-lateral*lateral*35.f);
    }
    if(input.kind<-3.5f && input.kind>-5.5f) // energy/creature sensor: receiver ring
        nucleus=1-smoothstep(.055f,.085f,abs(r-.25f));
    if(input.kind<-5.5f && input.kind>-6.5f)
        nucleus=(1-smoothstep(0.035f,0.065f,min(abs(input.local.x),abs(input.local.y))))*(1-smoothstep(0.18f,0.23f,r));
    if(input.kind<-6.5f) nucleus=exp(-pow((r-0.14f)*40,2.0f));
    if(input.kind<-7.5f && input.kind>-8.5f) {
        float fill=.06f+.19f*clamp(input.thrust.x,0.f,1.f);
        nucleus=1-smoothstep(fill,fill+.035f,r);
    }
    if(input.kind<-8.5f && input.kind>-9.5f) nucleus=exp(-pow((r-.28f)*40,2.f));
    if(input.kind<-9.5f && input.kind>-10.5f) nucleus=exp(-pow(input.local.y*40,2.f))*(1-smoothstep(.12f,.2f,r));
    if(input.kind<-10.5f && input.kind>-11.5f) nucleus=1-smoothstep(.05f,.08f,abs(r-.34f));
    if(input.kind<-11.5f && input.kind>-12.5f) nucleus=1-smoothstep(.04f,.07f,abs(r-.16f));
    float membrane = exp(-pow((r - 0.44f) * 30.0f, 2.0f));
    float3 color = input.color.rgb * (0.6f + 0.4f * core)
        + (nucleus * 0.6f + membrane * 0.22f) * input.color.rgb;
    // A short exhaust lobe shows the direction and magnitude of actual paid force.
    float magnitude=(input.kind<-2.5f && input.kind>-3.5f) ? length(input.thrust) : 0;
    float2 exhaust=-input.thrust/max(magnitude,0.00001f);
    float axial=dot(input.local,exhaust);
    float lateral=dot(input.local,float2(-exhaust.y,exhaust.x));
    float plume=smoothstep(0.35f,0.5f,axial)*(1.0f-smoothstep(0.55f,1.0f,axial))
        *exp(-lateral*lateral*70.0f)*min(magnitude,1.0f);
    return float4(color+input.color.rgb*plume, input.color.a * max(max(core, glow),plume));
}

float2 physicalDelta(float2 d, constant GpuPhysicsParams& p) {
    if(p.toroidal) {float2 size=float2(p.worldMaxX-p.worldMinX,p.worldMaxY-p.worldMinY);d-=size*round(d/size);}
    return d;
}
int2 spatialBin(float2 position, constant GpuPhysicsParams& p) {
    float2 origin=float2(p.worldMinX,p.worldMinY),size=float2(p.worldMaxX-p.worldMinX,p.worldMaxY-p.worldMinY);
    int2 dimensions=int2(p.gridX,p.gridY);
    int2 bin=int2(floor((position-origin)/size*float2(dimensions)));
    return p.toroidal ? (bin%dimensions+dimensions)%dimensions : clamp(bin,int2(0),dimensions-1);
}
kernel void buildSpatialGrid(const device GpuCellState* states [[buffer(0)]],
    device atomic_uint* heads [[buffer(1)]], device uint* links [[buffer(2)]],
    constant GpuPhysicsParams& p [[buffer(3)]], uint i [[thread_position_in_grid]]) {
    if(i>=p.cellCount)return;
    int2 b=spatialBin(states[i].position,p);
    links[i]=atomic_exchange_explicit(&heads[b.y*p.gridX+b.x],i,memory_order_relaxed);
}
kernel void physicsStep(
    const device GpuCellState* readState [[buffer(0)]],
    device GpuCellState* writeState [[buffer(1)]],
    const device GpuAdjacency* adjacency [[buffer(2)]],
    const device GpuNeighbor* neighbors [[buffer(3)]],
    constant GpuPhysicsParams& params [[buffer(4)]],
    const device packed_float2* actuation [[buffer(5)]],
    const device GpuAngle* angles [[buffer(6)]],
    const device uint* heads [[buffer(7)]], const device uint* links [[buffer(8)]],
    uint cellIndex [[thread_position_in_grid]])
{
    if (cellIndex >= params.cellCount) {
        return;
    }

    GpuCellState state = readState[cellIndex];
    float2 totalForce = float2(actuation[cellIndex]);
    GpuAdjacency cellAdjacency = adjacency[cellIndex];

    // Each cell thread reads a duplicated adjacency list and writes only its
    // own output state. This is the GPU equivalent of the CPU spring loop.
    for (uint offset = 0; offset < cellAdjacency.count; ++offset) {
        GpuNeighbor neighbor = neighbors[cellAdjacency.offset + offset];
        if (neighbor.neighborIndex >= params.cellCount) {
            continue;
        }
        GpuCellState other = readState[neighbor.neighborIndex];
        float2 displacement = physicalDelta(other.position - state.position,params);
        float distance = length(displacement);
        float2 direction = distance <= 0.000001f
            ? (cellIndex < neighbor.neighborIndex ? float2(1, 0) : float2(-1, 0))
            : displacement / distance;
        float relativeVelocity = dot(other.velocity - state.velocity, direction);
        float springForce = neighbor.stiffness * (distance - neighbor.restLength)
            + params.connectionDamping * relativeVelocity;
        totalForce += direction * springForce;
    }

    for(uint j=0;j<cellAdjacency.padding1;++j) {
        GpuAngle joint=angles[cellAdjacency.padding0+j];
        float2 u=physicalDelta(readState[joint.cellA].position-readState[joint.center].position,params);
        float2 v=physicalDelta(readState[joint.cellB].position-readState[joint.center].position,params);
        float uu=dot(u,u),vv=dot(v,v);
        if(uu<0.04f || vv<0.04f) continue;
        float2 pu=float2(-u.y,u.x),pv=float2(-v.y,v.x);
        float theta=atan2(u.x*v.y-u.y*v.x,dot(u,v));
        float error=atan2(sin(theta-joint.targetAngle),cos(theta-joint.targetAngle));
        float omega=dot(pv,readState[joint.cellB].velocity-readState[joint.center].velocity)/vv
            -dot(pu,readState[joint.cellA].velocity-readState[joint.center].velocity)/uu;
        float torque=clamp(joint.stiffness*error+0.35f*omega,-8.0f,8.0f);
        float2 fa=pu*(torque/uu),fb=pv*(-torque/vv);
        totalForce+=joint.slot==0 ? fa : joint.slot==2 ? fb : -(fa+fb);
    }

    // Bins are at least one interaction radius wide. Every cell has its own
    // linked-list slot: dense bins cannot overflow or silently lose neighbors.
    int2 bin=spatialBin(state.position,params),dimensions=int2(params.gridX,params.gridY);
    for(int dy=(dimensions.y<=2 ? 0 : -1);dy<=(dimensions.y<=2 ? dimensions.y-1 : 1);++dy)
    for(int dx=(dimensions.x<=2 ? 0 : -1);dx<=(dimensions.x<=2 ? dimensions.x-1 : 1);++dx) {
      int2 neighborBin=int2(dimensions.x<=2 ? dx : bin.x+dx,dimensions.y<=2 ? dy : bin.y+dy);
      if(params.toroidal)neighborBin=(neighborBin%dimensions+dimensions)%dimensions;
      else if(any(neighborBin<0) || any(neighborBin>=dimensions))continue;
      uint head=heads[neighborBin.y*params.gridX+neighborBin.x];
      for(uint otherIndex=head;otherIndex!=0xffffffffu;otherIndex=links[otherIndex]) {
        if (otherIndex == cellIndex) {
            continue;
        }
        float2 displacement = physicalDelta(state.position - readState[otherIndex].position,params);
        float distance = length(displacement);
        if (distance >= params.repulsionRadius) {
            continue;
        }
        bool connected = false;
        for (uint offset = 0; offset < cellAdjacency.count; ++offset) {
            if (neighbors[cellAdjacency.offset + offset].neighborIndex == otherIndex) {
                connected = true;
                break;
            }
        }
        if (connected) {
            continue;
        }

        float2 direction = distance <= 0.000001f ? (cellIndex < otherIndex ? float2(-1, 0) : float2(1, 0)) : displacement / distance;
        totalForce += direction * (params.repulsionStrength * (params.repulsionRadius - distance));
    }

    }
    float2 velocity = state.velocity + totalForce * params.dt;
    velocity *= max(0.0f, 1.0f - params.drag * params.dt);
    float2 position = state.position + velocity * params.dt;

    if(params.toroidal) {
        float2 origin=float2(params.worldMinX,params.worldMinY);
        float2 size=float2(params.worldMaxX-params.worldMinX,params.worldMaxY-params.worldMinY);
        position-=size*floor((position-origin)/size);
        writeState[cellIndex].position=position;writeState[cellIndex].velocity=velocity;return;
    }
    float minX = params.worldMinX + params.cellRadius;
    float maxX = params.worldMaxX - params.cellRadius;
    float minY = params.worldMinY + params.cellRadius;
    float maxY = params.worldMaxY - params.cellRadius;
    if (position.x < minX) {
        position.x = minX;
        if (velocity.x < 0.0f) {
            velocity.x *= -params.boundaryRestitution;
        }
    } else if (position.x > maxX) {
        position.x = maxX;
        if (velocity.x > 0.0f) {
            velocity.x *= -params.boundaryRestitution;
        }
    }
    if (position.y < minY) {
        position.y = minY;
        if (velocity.y < 0.0f) {
            velocity.y *= -params.boundaryRestitution;
        }
    } else if (position.y > maxY) {
        position.y = maxY;
        if (velocity.y > 0.0f) {
            velocity.y *= -params.boundaryRestitution;
        }
    }

    writeState[cellIndex].position = position;
    writeState[cellIndex].velocity = velocity;
}
