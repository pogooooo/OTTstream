// types.ts (기존 코드에 추가)

// 로그인/회원가입 요청 시 데이터 형식
export interface AuthPayload {
    nickname: string;
    password: string;
}

// 로그인/회원가입/수정/삭제 응답 형식
export interface AuthResponse {
    success: boolean;
    message: string;
    token?: string;
}

// 영상 목록 아이템 데이터 형식
export interface VideoItem {
    id: number;
    title: string;
    uploader: string;
    views: number;
    thumbnail: string;
    uploadDate: string;
}

// 영상 수정 요청 시 데이터 형식
export interface EditPayload {
    id: number;
    title: string;
}

// 영상 삭제 요청 시 데이터 형식 (URL 쿼리 사용)
export interface DeletePayload {
    id: number;
}
